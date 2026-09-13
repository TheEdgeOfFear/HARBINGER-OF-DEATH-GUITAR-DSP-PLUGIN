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

    void prepare(double sampleRate, int samplesPerBlock, int numChannels)
    {
        currentSampleRate = sampleRate;
        channels = numChannels;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
        spec.numChannels = static_cast<juce::uint32>(numChannels);

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

        octaveEngines.resize(static_cast<size_t>(numChannels));
        detuneVoices.resize(static_cast<size_t>(numChannels));
        for (int ch = 0; ch < numChannels; ++ch)
        {
            octaveEngines[static_cast<size_t>(ch)].prepare(sampleRate, samplesPerBlock);
            detuneVoices[static_cast<size_t>(ch)].prepare(sampleRate);
        }

        waveshaper.prepare(sampleRate, samplesPerBlock, numChannels, 4);
        chopModulator.prepare(sampleRate);

        dryScratchBuffer.setSize(numChannels, samplesPerBlock);
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
        const int numChans = std::min(buffer.getNumChannels(), channels);

        if (numSamples == 0 || numChans == 0)
            return;

        // Effect is active when Trash is engaged OR Chop is momentarily pressed/held
        const bool effectEngaged = trashActive || chopActive;

        // 1. If effect is disengaged: pass through clean signal
        if (!effectEngaged)
        {
            if (std::abs(inputGain - 1.0f) > 0.001f)
                buffer.applyGain(inputGain);

            if (std::abs(outputGain - 1.0f) > 0.001f)
                buffer.applyGain(outputGain);

            return;
        }

        // 2. Effect is ENGAGED: Process The Banshee Octave Fuzz & Chop
        dryScratchBuffer.setSize(numChans, numSamples, false, false, true);
        for (int ch = 0; ch < numChans; ++ch)
            dryScratchBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);

        // Input Conditioning (Input Gain)
        buffer.applyGain(inputGain);

        if (trashActive)
        {
            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            inputDcBlocker.process(context);
            inputTightFilter.process(context);

            // Polyphonic Octave Engine & Detune / Dissonance Engine
            for (int ch = 0; ch < numChans; ++ch)
            {
                auto& oct = octaveEngines[static_cast<size_t>(ch)];
                auto& det = detuneVoices[static_cast<size_t>(ch)];
                oct.setMode(octaveMode);

                float* chData = buffer.getWritePointer(ch);

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
            waveshaper.process(buffer, squareWave);

            // Signature Scooped V-Shaped EQ (750 Hz mid scoop + 3.8 kHz presence bite)
            juce::dsp::AudioBlock<float> eqBlock(buffer);
            juce::dsp::ProcessContextReplacing<float> eqContext(eqBlock);
            scoopFilter.process(eqContext);
            presenceFilter.process(eqContext);
        }

        // Signal-Chopping Tremolo (Chop Engine)
        chopModulator.process(buffer, chopActive, speedBpmSync, speedHz, syncDiv, hostBpm, hostPpq, isHostPlaying);

        // Master Dry/Wet Blending & Output Level
        const float wetGain = mix * outputGain;
        const float dryGain = (1.0f - mix) * outputGain;

        for (int ch = 0; ch < numChans; ++ch)
        {
            float* dest = buffer.getWritePointer(ch);
            const float* dry = dryScratchBuffer.getReadPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                dest[i] = (dry[i] * dryGain) + (dest[i] * wetGain);
            }
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
};

} // namespace HarbingerDSP
