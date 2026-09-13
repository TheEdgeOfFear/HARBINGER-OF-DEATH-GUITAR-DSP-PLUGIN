#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <cmath>

namespace HarbingerDSP
{

class DetuneVoice
{
public:
    DetuneVoice() = default;

    void prepare(double sampleRate, float phaseOffset = 0.0f)
    {
        currentSampleRate = sampleRate;
        bufferSize = 4096;
        bufferMask = bufferSize - 1;
        buffer.assign(static_cast<size_t>(bufferSize), 0.0f);
        writeIndex = 0;

        // Initialize 4 granular delay voices
        const float offsets[4] = { 0.0f, 0.25f, 0.5f, 0.75f };
        for (int i = 0; i < 4; ++i)
        {
            voices[i].phase = std::fmod(offsets[i] + phaseOffset + 1.0f, 1.0f);
            voices[i].lfoPhase = std::fmod(static_cast<float>(i) * 0.25f + phaseOffset + 1.0f, 1.0f);
        }
    }

    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex = 0;
    }

    // Process sample: blends detuned voices based on dissonance parameter (0 to 1)
    float processSample(float input, float dissonance)
    {
        if (dissonance <= 0.001f)
            return input;

        // Write to circular ring buffer
        buffer[static_cast<size_t>(writeIndex)] = input;

        // Window grain size (samples): ~25ms window for fast, responsive tracking
        const float grainSize = static_cast<float>(currentSampleRate) * 0.025f;
        const float minDelay = 48.0f;

        // Pitch shift scaling: 0.0 to 1.0 maps from ±10 cents to ±220 cents
        const float maxCents = 10.0f + (dissonance * dissonance * 210.0f);

        float sumVoices = 0.0f;

        // Direction/rate offsets for 4 voices
        const float pitchRatios[4] = {
            std::pow(2.0f,  maxCents / 1200.0f),
            std::pow(2.0f, -maxCents / 1200.0f),
            std::pow(2.0f, (maxCents * 0.60f) / 1200.0f),
            std::pow(2.0f, -(maxCents * 0.60f) / 1200.0f)
        };

        for (int i = 0; i < 4; ++i)
        {
            auto& v = voices[i];

            // Update LFO flutter for organic pitch sickness
            v.lfoPhase += (1.2f + static_cast<float>(i) * 0.7f) / static_cast<float>(currentSampleRate);
            if (v.lfoPhase >= 1.0f) v.lfoPhase -= 1.0f;
            const float flutter = std::sin(v.lfoPhase * juce::MathConstants<float>::twoPi) * (dissonance * 0.12f);

            // Phase step derived from pitch ratio
            const float speed = pitchRatios[i] + flutter;
            const float phaseStep = (1.0f - speed) / grainSize;
            v.phase += phaseStep;
            while (v.phase >= 1.0f) v.phase -= 1.0f;
            while (v.phase < 0.0f)  v.phase += 1.0f;

            // Dual crossfading read heads per voice for zero click
            const float p1 = v.phase;
            const float p2 = std::fmod(v.phase + 0.5f, 1.0f);

            const float w1 = 0.5f * (1.0f - std::cos(p1 * juce::MathConstants<float>::twoPi));
            const float w2 = 0.5f * (1.0f - std::cos(p2 * juce::MathConstants<float>::twoPi));

            const float delay1 = minDelay + p1 * grainSize;
            const float delay2 = minDelay + p2 * grainSize;

            const float s1 = readSampleHermite(static_cast<float>(writeIndex) - delay1);
            const float s2 = readSampleHermite(static_cast<float>(writeIndex) - delay2);

            sumVoices += (s1 * w1 + s2 * w2) * 0.25f;
        }

        // Advance write pointer
        writeIndex = (writeIndex + 1) & bufferMask;

        // Blend wet detuned discordance with direct signal
        const float wetGain = dissonance * 1.15f;
        const float dryGain = 1.0f - (dissonance * 0.35f);

        return (input * dryGain) + (sumVoices * wetGain);
    }

private:
    double currentSampleRate = 44100.0;
    int bufferSize = 4096;
    int bufferMask = 4095;
    std::vector<float> buffer;
    int writeIndex = 0;

    struct VoiceState
    {
        float phase = 0.0f;
        float lfoPhase = 0.0f;
    };

    VoiceState voices[4];

    // Cubic Hermite interpolation for pristine audio quality without high-frequency loss
    float readSampleHermite(float readPos) const
    {
        while (readPos < 0.0f)
            readPos += static_cast<float>(bufferSize);

        const int i1 = static_cast<int>(readPos) & bufferMask;
        const int i0 = (i1 - 1 + bufferSize) & bufferMask;
        const int i2 = (i1 + 1) & bufferMask;
        const int i3 = (i1 + 2) & bufferMask;

        const float frac = readPos - std::floor(readPos);

        const float y0 = buffer[static_cast<size_t>(i0)];
        const float y1 = buffer[static_cast<size_t>(i1)];
        const float y2 = buffer[static_cast<size_t>(i2)];
        const float y3 = buffer[static_cast<size_t>(i3)];

        const float c0 = y1;
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }
};

} // namespace HarbingerDSP
