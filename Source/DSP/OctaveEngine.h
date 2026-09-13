#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <algorithm>

namespace HarbingerDSP
{

enum class OctaveMode
{
    PlusOne = 0,   // +1 Octave
    PlusTwo = 1,   // +2 Octaves
    Both    = 2    // +1 and +2 Layered
};

class OctaveEngine
{
public:
    OctaveEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
        spec.numChannels = 1;

        preFilter.prepare(spec);
        preFilter.reset();
        preFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 750.0f, 0.707f);

        postFilter.prepare(spec);
        postFilter.reset();
        postFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, 1800.0f, 0.9f);

        dcBlocker1.prepare(spec);
        dcBlocker1.reset();
        dcBlocker1.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 60.0f, 0.707f);

        dcBlocker2.prepare(spec);
        dcBlocker2.reset();
        dcBlocker2.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 120.0f, 0.707f);

        attackCoeff = static_cast<float>(std::exp(-1.0 / (0.005 * sampleRate))); // fast 5ms attack
        decayCoeff  = static_cast<float>(std::exp(-1.0 / (0.120 * sampleRate))); // 120ms sustain bloom

        envelope = 0.0f;
    }

    void reset()
    {
        preFilter.reset();
        postFilter.reset();
        dcBlocker1.reset();
        dcBlocker2.reset();
        envelope = 0.0f;
    }

    void setMode(OctaveMode mode)
    {
        currentMode = mode;
    }

    // Process a single sample through the octave engine
    float processSample(float input, float octaveLevel)
    {
        if (octaveLevel <= 0.001f)
            return 0.0f;

        // 1. Isolate fundamental
        const float filtered = preFilter.processSample(input);

        // 2. Dynamic Envelope Follower for bloom
        const float inputAbs = std::abs(filtered);
        if (inputAbs > envelope)
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * inputAbs;
        else
            envelope = decayCoeff * envelope + (1.0f - decayCoeff) * inputAbs;

        // 3. Precision Full-Wave Rectification + DC block (+1 Octave)
        const float rect1 = std::abs(filtered);
        const float oct1 = dcBlocker1.processSample(rect1);

        // 4. Secondary Rectification (+2 Octaves)
        const float rect2 = std::abs(oct1);
        const float oct2 = dcBlocker2.processSample(rect2);

        // 5. Select Octave Signal
        float octSignal = 0.0f;
        switch (currentMode)
        {
            case OctaveMode::PlusOne:
                octSignal = oct1 * 2.2f;
                break;
            case OctaveMode::PlusTwo:
                octSignal = oct2 * 3.5f;
                break;
            case OctaveMode::Both:
                octSignal = (oct1 * 1.6f) + (oct2 * 2.2f);
                break;
        }

        // 6. Smooth high screech harmonics and apply octave level
        const float shapedOct = postFilter.processSample(octSignal);

        // High gain octave voice that sits prominently on top of the input signal
        return (shapedOct + octSignal * 0.4f) * (octaveLevel * 2.0f);
    }

private:
    double currentSampleRate = 44100.0;
    OctaveMode currentMode = OctaveMode::PlusOne;

    juce::dsp::IIR::Filter<float> preFilter;
    juce::dsp::IIR::Filter<float> postFilter;
    juce::dsp::IIR::Filter<float> dcBlocker1;
    juce::dsp::IIR::Filter<float> dcBlocker2;

    float attackCoeff = 0.99f;
    float decayCoeff  = 0.999f;
    float envelope    = 0.0f;
};

} // namespace HarbingerDSP
