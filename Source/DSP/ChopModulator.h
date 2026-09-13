#pragma once

#include <juce_core/juce_core.h>
#include <cmath>
#include <atomic>

namespace HarbingerDSP
{

enum class SyncDivision
{
    Quarter = 0,         // 1/4
    QuarterTriplet = 1,  // 1/4 T
    QuarterDotted = 2,   // 1/4 D
    Eighth = 3,          // 1/8
    EighthTriplet = 4,   // 1/8 T
    EighthDotted = 5,    // 1/8 D
    Sixteenth = 6,       // 1/16
    SixteenthTriplet = 7,// 1/16 T
    SixteenthDotted = 8, // 1/16 D
    ThirtySecond = 9,    // 1/32
    ThirtySecondTriplet = 10 // 1/32 T
};

inline const char* getSyncDivisionName(SyncDivision div)
{
    switch (div)
    {
        case SyncDivision::Quarter:          return "1/4";
        case SyncDivision::QuarterTriplet:   return "1/4 T";
        case SyncDivision::QuarterDotted:    return "1/4 D";
        case SyncDivision::Eighth:           return "1/8";
        case SyncDivision::EighthTriplet:    return "1/8 T";
        case SyncDivision::EighthDotted:     return "1/8 D";
        case SyncDivision::Sixteenth:        return "1/16";
        case SyncDivision::SixteenthTriplet: return "1/16 T";
        case SyncDivision::SixteenthDotted:  return "1/16 D";
        case SyncDivision::ThirtySecond:     return "1/32";
        case SyncDivision::ThirtySecondTriplet: return "1/32 T";
        default: return "1/8";
    }
}

class ChopModulator
{
public:
    ChopModulator() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        phase = 0.0f;
        smoothedGain = 1.0f;
        // ~1.5ms edge smoothing filter to prevent DC pops and click transients
        smoothCoeff = 1.0f - std::exp(-1.0f / (0.0015f * static_cast<float>(sampleRate)));
    }

    void reset()
    {
        phase = 0.0f;
        smoothedGain = 1.0f;
    }

    // Process a block of samples
    void process(juce::AudioBuffer<float>& buffer,
                 bool chopEngaged,
                 bool bpmSync,
                 float speedKnobHz,
                 SyncDivision division,
                 double bpm,
                 double hostPpqPosition = -1.0,
                 bool isHostPlaying = false)
    {
        // If Chop is not engaged, smoothly return gain to 1.0 (fully open)
        if (!chopEngaged)
        {
            const int numSamples = buffer.getNumSamples();
            const int numChannels = buffer.getNumChannels();

            if (smoothedGain < 0.999f)
            {
                for (int i = 0; i < numSamples; ++i)
                {
                    smoothedGain += (1.0f - smoothedGain) * smoothCoeff;
                    for (int ch = 0; ch < numChannels; ++ch)
                        buffer.getWritePointer(ch)[i] *= smoothedGain;
                }
            }
            currentLedValue.store(0.0f);
            return;
        }

        // 1. Calculate rate in Hz
        float rateHz = speedKnobHz;

        if (bpmSync)
        {
            if (bpm < 20.0 || bpm > 400.0)
                bpm = 120.0; // safe fallback

            double beats = 0.5; // default 1/8
            switch (division)
            {
                case SyncDivision::Quarter:             beats = 1.0; break;
                case SyncDivision::QuarterTriplet:      beats = 2.0 / 3.0; break;
                case SyncDivision::QuarterDotted:       beats = 1.5; break;
                case SyncDivision::Eighth:              beats = 0.5; break;
                case SyncDivision::EighthTriplet:       beats = 1.0 / 3.0; break;
                case SyncDivision::EighthDotted:        beats = 0.75; break;
                case SyncDivision::Sixteenth:           beats = 0.25; break;
                case SyncDivision::SixteenthTriplet:    beats = 1.0 / 6.0; break;
                case SyncDivision::SixteenthDotted:     beats = 0.375; break;
                case SyncDivision::ThirtySecond:        beats = 0.125; break;
                case SyncDivision::ThirtySecondTriplet: beats = 1.0 / 12.0; break;
            }

            rateHz = static_cast<float>(bpm / (60.0 * beats));
        }

        const float phaseIncrement = rateHz / static_cast<float>(currentSampleRate);
        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();

        float finalGain = smoothedGain;

        // If host is playing and we have valid musical position, sync per-sample to DAW timeline
        if (bpmSync && isHostPlaying && hostPpqPosition >= 0.0)
        {
            double beats = 0.5;
            switch (division)
            {
                case SyncDivision::Quarter:             beats = 1.0; break;
                case SyncDivision::QuarterTriplet:      beats = 2.0 / 3.0; break;
                case SyncDivision::QuarterDotted:       beats = 1.5; break;
                case SyncDivision::Eighth:              beats = 0.5; break;
                case SyncDivision::EighthTriplet:       beats = 1.0 / 3.0; break;
                case SyncDivision::EighthDotted:        beats = 0.75; break;
                case SyncDivision::Sixteenth:           beats = 0.25; break;
                case SyncDivision::SixteenthTriplet:    beats = 1.0 / 6.0; break;
                case SyncDivision::SixteenthDotted:     beats = 0.375; break;
                case SyncDivision::ThirtySecond:        beats = 0.125; break;
                case SyncDivision::ThirtySecondTriplet: beats = 1.0 / 12.0; break;
            }

            const double ppqStepPerSample = (bpm / 60.0) / currentSampleRate;
            double samplePpq = hostPpqPosition;

            for (int i = 0; i < numSamples; ++i)
            {
                const double beatInDivision = samplePpq / beats;
                const float currentPhase = static_cast<float>(beatInDivision - std::floor(beatInDivision));

                const float targetGain = (currentPhase < 0.5f) ? 1.0f : 0.0f;
                finalGain += (targetGain - finalGain) * smoothCoeff;

                for (int ch = 0; ch < numChannels; ++ch)
                    buffer.getWritePointer(ch)[i] *= finalGain;

                samplePpq += ppqStepPerSample;
            }

            // Keep internal phase continuous when transport stops
            phase = static_cast<float>(samplePpq / beats - std::floor(samplePpq / beats));
        }
        else
        {
            // Continuous phase accumulation: runs smoothly at rateHz (whether in Hz mode, or BPM sync when host stopped / Standalone)
            for (int i = 0; i < numSamples; ++i)
            {
                const float targetGain = (phase < 0.5f) ? 1.0f : 0.0f;
                finalGain += (targetGain - finalGain) * smoothCoeff;

                for (int ch = 0; ch < numChannels; ++ch)
                    buffer.getWritePointer(ch)[i] *= finalGain;

                phase += phaseIncrement;
                if (phase >= 1.0f)
                    phase -= 1.0f;
            }
        }

        smoothedGain = finalGain;
        currentLedValue.store(targetLedFromGain(finalGain));
    }

    float getVisualLedBrightness() const
    {
        return currentLedValue.load();
    }

private:
    double currentSampleRate = 44100.0;
    float phase = 0.0f;
    float smoothedGain = 1.0f;
    float smoothCoeff = 0.05f;
    std::atomic<float> currentLedValue{0.0f};

    static float targetLedFromGain(float gain)
    {
        return juce::jlimit(0.0f, 1.0f, gain);
    }
};

} // namespace HarbingerDSP
