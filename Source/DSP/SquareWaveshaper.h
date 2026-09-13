#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <algorithm>

namespace HarbingerDSP
{

class SquareWaveshaper
{
public:
    SquareWaveshaper() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels, int oversampleFactor = 4)
    {
        currentSampleRate = sampleRate;
        channels = numChannels;
        currentOversampleFactor = oversampleFactor;

        setupOversampling(oversampleFactor);

        lastSampleHold.assign(static_cast<size_t>(numChannels), 0.0f);
        decimatorCounter.assign(static_cast<size_t>(numChannels), 0);
    }

    void reset()
    {
        if (oversampling != nullptr)
            oversampling->reset();

        std::fill(lastSampleHold.begin(), lastSampleHold.end(), 0.0f);
        std::fill(decimatorCounter.begin(), decimatorCounter.end(), 0);
    }

    void setOversamplingFactor(int factor)
    {
        if (factor != currentOversampleFactor && (factor == 2 || factor == 4 || factor == 8))
        {
            currentOversampleFactor = factor;
            setupOversampling(factor);
        }
    }

    int getOversamplingFactor() const { return currentOversampleFactor; }

    // Process a multi-channel audio buffer through the waveshaper
    void process(juce::AudioBuffer<float>& buffer, float squareWaveAmount)
    {
        if (oversampling == nullptr)
            return;

        // Dynamic input pre-gain driven by squareWave knob
        const float drive = 1.0f + (squareWaveAmount * squareWaveAmount * 28.0f);

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::AudioBlock<float> oversampledBlock = oversampling->processSamplesUp(block);

        const int numChans = static_cast<int>(oversampledBlock.getNumChannels());
        const int numSamples = static_cast<int>(oversampledBlock.getNumSamples());

        for (int ch = 0; ch < numChans; ++ch)
        {
            float* channelData = oversampledBlock.getChannelPointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                const float x = channelData[i] * drive;

                // Corrupted square-wave fuzz with high harmonic density
                const float softFuzz = std::tanh(x * 1.5f);
                const float absX = std::abs(x);
                const float sgnX = (x >= 0.0f) ? 1.0f : -1.0f;
                const float hardSquare = sgnX * (1.0f - std::exp(-absX * (2.2f + squareWaveAmount * 30.0f)));

                const float shaped = (1.0f - squareWaveAmount) * softFuzz + squareWaveAmount * hardSquare;
                channelData[i] = std::clamp(shaped * 0.95f, -1.0f, 1.0f);
            }
        }

        oversampling->processSamplesDown(block);
    }

private:
    double currentSampleRate = 44100.0;
    int channels = 2;
    int currentOversampleFactor = 4;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;

    std::vector<float> lastSampleHold;
    std::vector<int> decimatorCounter;

    void setupOversampling(int factor)
    {
        size_t order = 2; // 2^2 = 4x
        if (factor == 2) order = 1;
        else if (factor == 8) order = 3;

        oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
            static_cast<size_t>(channels),
            order,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            true, // isMaxQuality
            false // integerLatency
        );

        oversampling->reset();
    }
};

} // namespace HarbingerDSP
