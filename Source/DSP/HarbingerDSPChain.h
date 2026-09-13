#pragma once

#include "OctaveEngine.h"
#include "DetuneVoice.h"
#include "SquareWaveshaper.h"
#include "ChopModulator.h"
#include <juce_dsp/juce_dsp.h>

namespace HarbingerDSP
{

class HarbingerDSPChain
{
public:
    HarbingerDSPChain() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels = 2)
    {
        juce::ignoreUnused(numChannels);
        currentSampleRate = sampleRate;
        channels = 2;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
        spec.numChannels = 2;

        // Input conditioning filters
        inputDcBlocker.prepare(spec);
        inputDcBlocker.reset();
        *inputDcBlocker.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);

        inputTightFilter.prepare(spec);
        inputTightFilter.reset();
        // 55 Hz high-pass to tighten 7/8 string low rumble before extreme waveshaping
        *inputTightFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 55.0f, 0.707f);

        // V-shaped Scooped EQ (Mid notch at 750 Hz + Presence peak at 3.8 kHz)
        scoopFilter.prepare(spec);
        scoopFilter.reset();
        *scoopFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 750.0f, 1.0f, juce::Decibels::decibelsToGain(-5.5f));

        presenceFilter.prepare(spec);
        presenceFilter.reset();
        *presenceFilter.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 3800.0f, 0.8f, juce::Decibels::decibelsToGain(+3.5f));

        octaveEngines.resize(2);
        detuneVoices.resize(2);
        for (int ch = 0; ch < 2; ++ch)
        {
            octaveEngines[static_cast<size_t>(ch)].prepare(sampleRate, samplesPerBlock);
            detuneVoices[static_cast<size_t>(ch)].prepare(sampleRate, ch == 1 ? 0.33f : 0.0f);
        }

        waveshaper.prepare(sampleRate, samplesPerBlock, 2, 4);
        chopModulator.prepare(sampleRate);

        dryScratchBuffer.setSize(2, samplesPerBlock);
        monoToStereoScratchBuffer.setSize(2, samplesPerBlock);
    }

    void reset()
    {
        inputDcBlocker.reset();
        inputTightFilter.reset();
        scoopFilter.reset();
        presenceFilter.reset();
        for (auto& o : octaveEngines) o.reset();
        for (auto& d : detuneVoices) d.reset();
        waveshaper.reset();
        chopModulator.reset();
    }

    void setOversamplingFactor(int factor)
    {
        waveshaper.setOversamplingFactor(factor);
    }

    int getOversamplingFactor() const
    {
        return waveshaper.getOversamplingFactor();
    }

    float getChopLedBrightness() const
    {
        return chopModulator.getVisualLedBrightness();
    }

    void process(juce::AudioBuffer<float>& buffer,
                 float inputGain,
                 float dissonance,
                 float squareWave,
                 float octaveLevel,
                 OctaveMode octaveMode,
                 float speedHz,
                 bool speedBpmSync,
                 SyncDivision syncDiv,
                 bool trashActive,
                 bool chopActive,
                 float mix,
                 float outputGain,
                 double hostBpm,
                 double hostPpq,
                 bool isHostPlaying = false)
    {
        const int numSamples = buffer.getNumSamples();
        const int bufChannels = buffer.getNumChannels();

        if (numSamples == 0 || bufChannels == 0)
            return;

        const bool isMonoInput = (bufChannels == 1);
        juce::AudioBuffer<float>& workBuffer = isMonoInput ? monoToStereoScratchBuffer : buffer;

        if (isMonoInput)
        {
            if (monoToStereoScratchBuffer.getNumSamples() < numSamples)
                monoToStereoScratchBuffer.setSize(2, numSamples, false, false, true);

            monoToStereoScratchBuffer.copyFrom(0, 0, buffer.getReadPointer(0), numSamples);
            monoToStereoScratchBuffer.copyFrom(1, 0, buffer.getReadPointer(0), numSamples);
        }

        // Effect is active when Trash is engaged OR Chop is momentarily pressed/held
        const bool effectEngaged = trashActive || chopActive;

        // 1. If effect is disengaged: pass through clean signal
        if (!effectEngaged)
        {
            if (std::abs(inputGain - 1.0f) > 0.001f)
                workBuffer.applyGain(inputGain);

            if (std::abs(outputGain - 1.0f) > 0.001f)
                workBuffer.applyGain(outputGain);

            if (isMonoInput)
            {
                buffer.copyFrom(0, 0, monoToStereoScratchBuffer.getReadPointer(0), numSamples);
            }
            else if (bufChannels > 2)
            {
                for (int ch = 2; ch < bufChannels; ++ch)
                    buffer.clear(ch, 0, numSamples);
            }
            return;
        }

        // 2. Effect is ENGAGED: Process The Banshee Octave Fuzz & Chop
        if (dryScratchBuffer.getNumSamples() < numSamples)
            dryScratchBuffer.setSize(2, numSamples, false, false, true);

        dryScratchBuffer.copyFrom(0, 0, workBuffer.getReadPointer(0), numSamples);
        dryScratchBuffer.copyFrom(1, 0, workBuffer.getReadPointer(1), numSamples);

        // Input Conditioning (Input Gain)
        workBuffer.applyGain(inputGain);

        if (trashActive)
        {
            float* channelDataPointers[2] = {
                workBuffer.getWritePointer(0),
                workBuffer.getWritePointer(1)
            };
            juce::dsp::AudioBlock<float> block(channelDataPointers, 2, static_cast<size_t>(numSamples));
            juce::dsp::ProcessContextReplacing<float> context(block);
            inputDcBlocker.process(context);
            inputTightFilter.process(context);

            // Polyphonic Octave Engine & Detune / Dissonance Engine
            for (int ch = 0; ch < 2; ++ch)
            {
                auto& oct = octaveEngines[static_cast<size_t>(ch)];
                auto& det = detuneVoices[static_cast<size_t>(ch)];
                oct.setMode(octaveMode);

                float* chData = workBuffer.getWritePointer(ch);

                for (int i = 0; i < numSamples; ++i)
                {
                    const float in = chData[i];

                    // Polyphonic octave voice (+1, +2, or Both)
                    const float octVoice = oct.processSample(in, octaveLevel);

                    // Combine fundamental with octave voice
                    const float combined = in + octVoice;

                    // Pass into multi-voice detuner for dissonance
                    chData[i] = det.processSample(combined, dissonance);
                }
            }

            // The Banshee Corrupted Square-Wave Fuzz (4x Oversampled)
            // Create a temporary 2-channel audio buffer wrapper for the waveshaper
            juce::AudioBuffer<float> twoChannelWrapper(workBuffer.getArrayOfWritePointers(), 2, numSamples);
            waveshaper.process(twoChannelWrapper, squareWave);

            // Signature Scooped V-Shaped EQ (750 Hz mid scoop + 3.8 kHz presence bite)
            juce::dsp::AudioBlock<float> eqBlock(channelDataPointers, 2, static_cast<size_t>(numSamples));
            juce::dsp::ProcessContextReplacing<float> eqContext(eqBlock);
            scoopFilter.process(eqContext);
            presenceFilter.process(eqContext);
        }

        // Signal-Chopping Tremolo (Chop Engine)
        juce::AudioBuffer<float> chopWrapper(workBuffer.getArrayOfWritePointers(), 2, numSamples);
        chopModulator.process(chopWrapper, chopActive, speedBpmSync, speedHz, syncDiv, hostBpm, hostPpq, isHostPlaying);

        // Master Dry/Wet Blending & Output Level
        const float wetGain = mix * outputGain;
        const float dryGain = (1.0f - mix) * outputGain;

        for (int ch = 0; ch < 2; ++ch)
        {
            float* dest = workBuffer.getWritePointer(ch);
            const float* dry = dryScratchBuffer.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                dest[i] = (dry[i] * dryGain) + (dest[i] * wetGain);
            }
        }

        if (isMonoInput)
        {
            buffer.copyFrom(0, 0, monoToStereoScratchBuffer.getReadPointer(0), numSamples);
        }
        else if (bufChannels > 2)
        {
            for (int ch = 2; ch < bufChannels; ++ch)
                buffer.clear(ch, 0, numSamples);
        }
    }

private:
    double currentSampleRate = 44100.0;
    int channels = 2;

    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> inputDcBlocker;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> inputTightFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> scoopFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> presenceFilter;

    std::vector<OctaveEngine> octaveEngines;
    std::vector<DetuneVoice> detuneVoices;
    SquareWaveshaper waveshaper;
    ChopModulator chopModulator;

    juce::AudioBuffer<float> dryScratchBuffer;
    juce::AudioBuffer<float> monoToStereoScratchBuffer;
};

} // namespace HarbingerDSP
