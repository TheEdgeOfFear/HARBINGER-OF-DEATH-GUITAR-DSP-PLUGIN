#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "HarbingerLookAndFeel.h"
#include "MidiMappingModal.h"

class HarbingerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      public juce::Timer,
                                      public juce::ComboBox::Listener,
                                      public juce::Button::Listener
{
public:
    explicit HarbingerAudioProcessorEditor(HarbingerAudioProcessor&);
    ~HarbingerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked(juce::Button* button) override;

private:
    HarbingerAudioProcessor& audioProcessor;
    HarbingerLookAndFeel harbingerLookAndFeel;

    // --- Top Bar Preset Browser & Master Section ---
    juce::TextButton prevPresetButton{"<"};
    juce::ComboBox categoryBox;
    juce::ComboBox presetBox;
    juce::TextButton nextPresetButton{">"};
    juce::TextButton savePresetButton{"SAVE"};
    juce::TextButton deletePresetButton{"DEL"};

    BrutalKnob inputGainSlider{1.0};
    juce::Label inputGainLabel;
    BrutalKnob mixSlider{1.0};
    juce::Label mixLabel;
    BrutalKnob outputGainSlider{1.0};
    juce::Label outputGainLabel;

    juce::ComboBox oversampleBox;
    juce::Label oversampleLabel;
    juce::TextButton midiMenuButton{"MIDI MAP"};
    BrutalPowerButton powerButton{"POWER ON"};

    // --- Sub-Banner / Status Ribbon ---
    juce::Label bannerLabel;

    // --- Pedal Faceplate Controls ---
    // 4 Main Massive Knobs
    BrutalKnob dissonanceSlider{0.15};
    juce::Label dissonanceLabel;

    BrutalKnob squareWaveSlider{0.85};
    juce::Label squareWaveLabel;

    BrutalKnob octaveSlider{0.65};
    juce::Label octaveLabel;

    BrutalKnob speedSlider{8.0};
    juce::Label speedLabel;

    // 2 3-Way Mode Selectors
    ThreeWaySwitch octaveModeSwitch{"+1", "+2", "Both"};
    juce::Label octaveModeLabel;

    ThreeWaySwitch trashModeSwitch{"Latching", "Momentary", "Tap+Hold"};
    juce::Label trashModeLabel;

    // Chop Rhythmic Sync Controls
    BrutalSlideToggle speedSyncToggle{"BPM SYNC"};
    juce::ComboBox syncDivisionBox;

    // 2 Heavy-Duty Footswitches & Link Toggle
    FootswitchComponent chopFootswitch{"CHOP"};
    FootswitchComponent trashFootswitch{"TRASH"};
    BrutalSlideToggle stompLinkToggle{"LINK"};

    // Modal Overlays
    std::unique_ptr<MidiMappingModal> midiModal;

    // APVTS Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> inGainAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> outGainAttachment;
    std::unique_ptr<SliderAttachment> dissonanceAttachment;
    std::unique_ptr<SliderAttachment> squareWaveAttachment;
    std::unique_ptr<SliderAttachment> octaveAttachment;
    std::unique_ptr<SliderAttachment> speedAttachment;
    std::unique_ptr<ButtonAttachment> speedSyncAttachment;
    std::unique_ptr<ComboBoxAttachment> syncDivAttachment;
    std::unique_ptr<ComboBoxAttachment> oversampleAttachment;
    std::unique_ptr<ButtonAttachment> powerAttachment;
    std::unique_ptr<ButtonAttachment> stompLinkAttachment;

    std::vector<std::string> categories;
    std::vector<int> filteredIndices;
    bool lastBpmSyncState = false;
    double lastKnownBpm = 120.0;

    void populateCategories();
    void syncPresetUIFromProcessorState();
    void updatePresetDropdown();
    void openMidiContextMenu(const std::string& paramId, const std::string& dispName);
    void promptSaveUserPreset();
    void selectPreviousPreset();
    void selectNextPreset();

    void drawHexScrew(juce::Graphics& g, float cx, float cy);
    void loadBackgroundImage();
    void loadTopBarImage();

    juce::Image backgroundImage;
    juce::Image topBarImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarbingerAudioProcessorEditor)
};
