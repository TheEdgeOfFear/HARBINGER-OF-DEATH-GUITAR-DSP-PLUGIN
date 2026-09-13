#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "DSP/HarbingerDSPChain.h"
#include "MidiManager.h"
#include "PresetManager.h"
#include <atomic>

class HarbingerAudioProcessor : public juce::AudioProcessor
{
public:
    HarbingerAudioProcessor();
    ~HarbingerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }
    MidiManager& getMidiManager() { return midiManager; }
    PresetManager& getPresetManager() { return presetManager; }

    // Footswitch interaction handlers
    void handleChopPointerState(bool isDown);
    void handleTrashPointerState(bool isDown);
    void handleTrashClick();

    bool isTrashEngineActive() const { return trashEngineActive.load(); }
    bool isChopEngineActive() const { return chopEngineActive.load(); }
    bool isPluginEnabled() const;
    float getChopLedBrightness() const { return dspChain.getChopLedBrightness(); }
    double getHostBpm() const { return currentHostBpm.load(); }
    bool getIsHostPlaying() const { return hostIsPlaying.load(); }

    void applyPreset(int globalIndex);
    int getSelectedPresetIndex() const { return currentPresetIndex.load(); }

private:
    juce::AudioProcessorValueTreeState parameters;
    MidiManager midiManager;
    PresetManager presetManager;
    HarbingerDSP::HarbingerDSPChain dspChain;

    std::atomic<int> currentPresetIndex{0};

    // Footswitch state machine
    std::atomic<bool> trashLatched{true};
    std::atomic<bool> trashMomentary{false};
    std::atomic<bool> trashEngineActive{true};
    std::atomic<bool> chopEngineActive{false};
    std::atomic<bool> guiChopHeld{false};
    std::atomic<bool> midiChopHeld{false};
    std::atomic<bool> midiTrashHeld{false};
    std::atomic<double> currentHostBpm{120.0};
    std::atomic<bool> hostIsPlaying{false};

    juce::int64 trashPressTimestamp{0};

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarbingerAudioProcessor)
};
