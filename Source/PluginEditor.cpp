#include "PluginEditor.h"
#include <BinaryData.h>

HarbingerAudioProcessorEditor::HarbingerAudioProcessorEditor(HarbingerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&harbingerLookAndFeel);
    setResizable(false, false);
    loadBackgroundImage();
    loadTopBarImage();
    setSize(1080, 720);

    // ==========================================
    // 0. MASTER POWER / ENABLE BUTTON
    // ==========================================
    powerButton.setClickingTogglesState(true);
    powerButton.setToggleState(audioProcessor.isPluginEnabled(), juce::dontSendNotification);
    powerButton.setButtonText(audioProcessor.isPluginEnabled() ? "POWER ON" : "BYPASS");
    powerButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff220a0e));
    powerButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff8a0a14));
    powerButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    powerButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff8a94a6));
    powerButton.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("pluginEnabled", "Plugin Enable / Bypass");
    };
    powerButton.onClick = [this]() {
        const bool on = powerButton.getToggleState();
        powerButton.setButtonText(on ? "POWER ON" : "BYPASS");
        repaint();
    };
    addAndMakeVisible(powerButton);

    // ==========================================
    // 1. TOP PRESET BROWSER SECTION
    // ==========================================
    prevPresetButton.addListener(this);
    addAndMakeVisible(prevPresetButton);

    categoryBox.addListener(this);
    addAndMakeVisible(categoryBox);

    presetBox.addListener(this);
    addAndMakeVisible(presetBox);

    nextPresetButton.addListener(this);
    addAndMakeVisible(nextPresetButton);

    savePresetButton.addListener(this);
    addAndMakeVisible(savePresetButton);

    deletePresetButton.addListener(this);
    addAndMakeVisible(deletePresetButton);

    // ==========================================
    // 2. TOP RIGHT MASTER CONTROLS
    // ==========================================
    inputGainSlider.setRange(0.0, 2.0, 0.01);
    inputGainSlider.setValue(1.0);
    inputGainSlider.setDefaultResetValue(1.0);
    inputGainSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("inputGain", "Input Gain");
    };
    addAndMakeVisible(inputGainSlider);

    inputGainLabel.setText("INPUT", juce::dontSendNotification);
    inputGainLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(inputGainLabel);

    mixSlider.setRange(0.0, 1.0, 0.01);
    mixSlider.setValue(1.0);
    mixSlider.setDefaultResetValue(1.0);
    mixSlider.setTextValueSuffix(" %");
    mixSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("mix", "Master Mix");
    };
    addAndMakeVisible(mixSlider);

    mixLabel.setText("MIX", juce::dontSendNotification);
    mixLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(mixLabel);

    outputGainSlider.setRange(0.0, 2.0, 0.01);
    outputGainSlider.setValue(1.0);
    outputGainSlider.setDefaultResetValue(1.0);
    outputGainSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("outputGain", "Output Gain");
    };
    addAndMakeVisible(outputGainSlider);

    outputGainLabel.setText("OUTPUT", juce::dontSendNotification);
    outputGainLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(outputGainLabel);

    // Oversampling dropdown (2x, 4x, 8x)
    oversampleBox.addItem("2X OS", 1);
    oversampleBox.addItem("4X OS", 2);
    oversampleBox.addItem("8X OS", 3);
    addAndMakeVisible(oversampleBox);

    oversampleLabel.setText("HQ OS", juce::dontSendNotification);
    oversampleLabel.setFont(juce::FontOptions(9.0f, juce::Font::bold));
    oversampleLabel.setJustificationType(juce::Justification::centred);
    oversampleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(oversampleLabel);

    midiMenuButton.addListener(this);
    addAndMakeVisible(midiMenuButton);

    // ==========================================
    // 3. SUB-BANNER STATUS RIBBON
    // ==========================================
    bannerLabel.setText("THE EDGE OF FEAR // HARBINGER OF DEATH // CHOPPED & TRASHED DIGITAL OCTAVE FUZZ", juce::dontSendNotification);
    bannerLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold | juce::Font::italic));
    bannerLabel.setJustificationType(juce::Justification::centred);
    bannerLabel.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    bannerLabel.setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    bannerLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff2233));
    addAndMakeVisible(bannerLabel);

    // ==========================================
    // 4. MAIN PEDAL CONTROLS
    // ==========================================
    // Dissonance
    dissonanceSlider.setRange(0.0, 1.0, 0.01);
    dissonanceSlider.setValue(0.15);
    dissonanceSlider.setDefaultResetValue(0.15);
    dissonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 68, 18);
    dissonanceSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("dissonance", "Dissonance");
    };
    addAndMakeVisible(dissonanceSlider);

    dissonanceLabel.setText("DISSONANCE", juce::dontSendNotification);
    dissonanceLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    dissonanceLabel.setJustificationType(juce::Justification::centred);
    dissonanceLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(dissonanceLabel);

    // Square Wave
    squareWaveSlider.setRange(0.0, 1.0, 0.01);
    squareWaveSlider.setValue(0.85);
    squareWaveSlider.setDefaultResetValue(0.85);
    squareWaveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 68, 18);
    squareWaveSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("squareWave", "Square Wave");
    };
    addAndMakeVisible(squareWaveSlider);

    squareWaveLabel.setText("SQUARE WAVE", juce::dontSendNotification);
    squareWaveLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    squareWaveLabel.setJustificationType(juce::Justification::centred);
    squareWaveLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(squareWaveLabel);

    // Octave
    octaveSlider.setRange(0.0, 1.0, 0.01);
    octaveSlider.setValue(0.65);
    octaveSlider.setDefaultResetValue(0.65);
    octaveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 68, 18);
    octaveSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("octaveLevel", "Octave Level");
    };
    addAndMakeVisible(octaveSlider);

    octaveLabel.setText("OCTAVE", juce::dontSendNotification);
    octaveLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    octaveLabel.setJustificationType(juce::Justification::centred);
    octaveLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(octaveLabel);

    // Speed
    speedSlider.setRange(0.5, 35.0, 0.01);
    speedSlider.setValue(8.0);
    speedSlider.setDefaultResetValue(8.0);
    speedSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 86, 18);
    speedSlider.textFromValueFunction = [this](double val) -> juce::String {
        if (speedSyncToggle.getToggleState())
        {
            const int divIdx = juce::jlimit(0, 10, syncDivisionBox.getSelectedItemIndex());
            const char* divName = HarbingerDSP::getSyncDivisionName(static_cast<HarbingerDSP::SyncDivision>(divIdx));
            const double bpm = audioProcessor.getHostBpm();
            double beats = 0.5;
            switch (static_cast<HarbingerDSP::SyncDivision>(divIdx))
            {
                case HarbingerDSP::SyncDivision::Quarter:             beats = 1.0; break;
                case HarbingerDSP::SyncDivision::QuarterTriplet:      beats = 2.0 / 3.0; break;
                case HarbingerDSP::SyncDivision::QuarterDotted:       beats = 1.5; break;
                case HarbingerDSP::SyncDivision::Eighth:              beats = 0.5; break;
                case HarbingerDSP::SyncDivision::EighthTriplet:       beats = 1.0 / 3.0; break;
                case HarbingerDSP::SyncDivision::EighthDotted:        beats = 0.75; break;
                case HarbingerDSP::SyncDivision::Sixteenth:           beats = 0.25; break;
                case HarbingerDSP::SyncDivision::SixteenthTriplet:    beats = 1.0 / 6.0; break;
                case HarbingerDSP::SyncDivision::SixteenthDotted:     beats = 0.375; break;
                case HarbingerDSP::SyncDivision::ThirtySecond:        beats = 0.125; break;
                case HarbingerDSP::SyncDivision::ThirtySecondTriplet: beats = 1.0 / 12.0; break;
            }
            const double safeBpm = (bpm > 20.0 && bpm < 400.0) ? bpm : 120.0;
            const double hz = safeBpm / (60.0 * beats);
            return juce::String(divName) + " (" + juce::String(hz, 1) + " Hz)";
        }
        return juce::String(val, 2) + " Hz";
    };

    speedSlider.valueFromTextFunction = [this](const juce::String& text) -> double {
        if (speedSyncToggle.getToggleState())
            return speedSlider.getValue();
        return text.upToFirstOccurrenceOf(" Hz", false, false).getDoubleValue();
    };

    speedSlider.onValueChange = [this]() {
        if (speedSyncToggle.getToggleState())
        {
            const double norm = (speedSlider.getValue() - speedSlider.getMinimum()) / (speedSlider.getMaximum() - speedSlider.getMinimum());
            const int divIdx = juce::jlimit(0, 10, static_cast<int>(std::round(norm * 10.0)));
            if (syncDivisionBox.getSelectedItemIndex() != divIdx)
            {
                if (auto* param = audioProcessor.getAPVTS().getParameter("syncDivision"))
                    param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(divIdx)));
                syncDivisionBox.setSelectedItemIndex(divIdx, juce::dontSendNotification);
            }
            speedSlider.updateText();
        }
    };

    speedSlider.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("speedHz", "Chop Speed");
    };
    addAndMakeVisible(speedSlider);

    speedLabel.setText("SPEED", juce::dontSendNotification);
    speedLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    speedLabel.setJustificationType(juce::Justification::centred);
    speedLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(speedLabel);

    // ==========================================
    // 5. MODE SELECTORS
    // ==========================================
    octaveModeSwitch.onChange = [this](int idx) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("octaveMode"))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    octaveModeSwitch.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("octaveMode", "Octave Mode");
    };
    addAndMakeVisible(octaveModeSwitch);

    octaveModeLabel.setText("OCTAVE SELECT", juce::dontSendNotification);
    octaveModeLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    octaveModeLabel.setJustificationType(juce::Justification::centred);
    octaveModeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff2233));
    addAndMakeVisible(octaveModeLabel);

    trashModeSwitch.onChange = [this](int idx) {
        if (auto* param = audioProcessor.getAPVTS().getParameter("trashMode"))
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(idx)));
    };
    trashModeSwitch.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("trashMode", "Trash Switch Mode");
    };
    addAndMakeVisible(trashModeSwitch);

    trashModeLabel.setText("TRASH MODE", juce::dontSendNotification);
    trashModeLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    trashModeLabel.setJustificationType(juce::Justification::centred);
    trashModeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff2233));
    addAndMakeVisible(trashModeLabel);

    // Speed BPM Sync controls
    speedSyncToggle.addListener(this);
    speedSyncToggle.onClick = [this]() {
        const bool isSync = speedSyncToggle.getToggleState();
        speedLabel.setText(isSync ? "SPEED [SYNC]" : "SPEED", juce::dontSendNotification);
        speedLabel.setColour(juce::Label::textColourId, isSync ? juce::Colour(0xffff2233) : juce::Colours::white);
        if (isSync)
        {
            const int divIdx = syncDivisionBox.getSelectedItemIndex();
            if (divIdx >= 0 && divIdx <= 10)
            {
                const double targetVal = speedSlider.getMinimum() + (static_cast<double>(divIdx) / 10.0) * (speedSlider.getMaximum() - speedSlider.getMinimum());
                speedSlider.setValue(targetVal, juce::dontSendNotification);
            }
        }
        speedSlider.updateText();
    };
    speedSyncToggle.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("speedBpmSync", "Speed BPM Sync");
    };
    addAndMakeVisible(speedSyncToggle);

    syncDivisionBox.addItem("1/4", 1);
    syncDivisionBox.addItem("1/4 T", 2);
    syncDivisionBox.addItem("1/4 D", 3);
    syncDivisionBox.addItem("1/8", 4);
    syncDivisionBox.addItem("1/8 T", 5);
    syncDivisionBox.addItem("1/8 D", 6);
    syncDivisionBox.addItem("1/16", 7);
    syncDivisionBox.addItem("1/16 T", 8);
    syncDivisionBox.addItem("1/16 D", 9);
    syncDivisionBox.addItem("1/32", 10);
    syncDivisionBox.addItem("1/32 T", 11);
    syncDivisionBox.onChange = [this]() {
        if (speedSyncToggle.getToggleState())
        {
            const int divIdx = syncDivisionBox.getSelectedItemIndex();
            if (divIdx >= 0 && divIdx <= 10)
            {
                const double targetVal = speedSlider.getMinimum() + (static_cast<double>(divIdx) / 10.0) * (speedSlider.getMaximum() - speedSlider.getMinimum());
                if (std::abs(speedSlider.getValue() - targetVal) > 0.05)
                    speedSlider.setValue(targetVal, juce::dontSendNotification);
            }
            speedSlider.updateText();
        }
    };
    addAndMakeVisible(syncDivisionBox);

    // ==========================================
    // 6. HEAVY-DUTY FOOTSWITCHES
    // ==========================================
    // CHOP: Momentary footswitch (active while held, forces Trash active)
    chopFootswitch.onPointerState = [this](bool isDown) {
        audioProcessor.handleChopPointerState(isDown);
    };
    chopFootswitch.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("chopStomp", "Chop Footswitch");
    };
    addAndMakeVisible(chopFootswitch);

    // TRASH: 3-mode footswitch (Latching, Momentary, Tap+Hold)
    trashFootswitch.onPointerState = [this](bool isDown) {
        audioProcessor.handleTrashPointerState(isDown);
    };
    trashFootswitch.onClick = [this]() {
        audioProcessor.handleTrashClick();
    };
    trashFootswitch.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("trashStomp", "Trash Footswitch");
    };
    addAndMakeVisible(trashFootswitch);

    // LINK: Slide switch to link Chop and Trash footswitches
    stompLinkToggle.onRightClick = [this](const juce::MouseEvent&) {
        openMidiContextMenu("linkStomps", "Link Footswitches");
    };
    addAndMakeVisible(stompLinkToggle);

    // ==========================================
    // 7. APVTS ATTACHMENTS
    // ==========================================
    auto& apvts = audioProcessor.getAPVTS();
    inGainAttachment     = std::make_unique<SliderAttachment>(apvts, "inputGain", inputGainSlider);
    mixAttachment        = std::make_unique<SliderAttachment>(apvts, "mix", mixSlider);
    outGainAttachment    = std::make_unique<SliderAttachment>(apvts, "outputGain", outputGainSlider);
    dissonanceAttachment = std::make_unique<SliderAttachment>(apvts, "dissonance", dissonanceSlider);
    squareWaveAttachment = std::make_unique<SliderAttachment>(apvts, "squareWave", squareWaveSlider);
    octaveAttachment     = std::make_unique<SliderAttachment>(apvts, "octaveLevel", octaveSlider);
    speedAttachment      = std::make_unique<SliderAttachment>(apvts, "speedHz", speedSlider);
    speedSyncAttachment  = std::make_unique<ButtonAttachment>(apvts, "speedBpmSync", speedSyncToggle);
    syncDivAttachment    = std::make_unique<ComboBoxAttachment>(apvts, "syncDivision", syncDivisionBox);
    oversampleAttachment = std::make_unique<ComboBoxAttachment>(apvts, "oversampling", oversampleBox);
    powerAttachment      = std::make_unique<ButtonAttachment>(apvts, "pluginEnabled", powerButton);
    stompLinkAttachment  = std::make_unique<ButtonAttachment>(apvts, "linkStomps", stompLinkToggle);

    // Initialize 3-way switches
    octaveModeSwitch.setSelectedIndex(static_cast<int>(apvts.getRawParameterValue("octaveMode")->load()), juce::dontSendNotification);
    trashModeSwitch.setSelectedIndex(static_cast<int>(apvts.getRawParameterValue("trashMode")->load()), juce::dontSendNotification);

    syncPresetUIFromProcessorState();

    startTimerHz(40); // 40 FPS GUI animations and state polling
}

HarbingerAudioProcessorEditor::~HarbingerAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void HarbingerAudioProcessorEditor::loadTopBarImage()
{
    if constexpr (BinaryData::HARBINGER_TOP_BAR_jpgSize > 0)
    {
        topBarImage = juce::ImageFileFormat::loadFrom(BinaryData::HARBINGER_TOP_BAR_jpg,
                                                      static_cast<size_t>(BinaryData::HARBINGER_TOP_BAR_jpgSize));
    }

    if (topBarImage.isNull())
    {
        juce::File f("C:/Coding/Tunings VST3/HARBINGER OF DEATH/Top Bar Image/HARBINGER_TOP_BAR.jpg");
        if (f.existsAsFile())
            topBarImage = juce::ImageFileFormat::loadFrom(f);
    }
}

void HarbingerAudioProcessorEditor::loadBackgroundImage()
{
    if constexpr (BinaryData::Background_jpgSize > 0)
    {
        backgroundImage = juce::ImageFileFormat::loadFrom(BinaryData::Background_jpg,
                                                          static_cast<size_t>(BinaryData::Background_jpgSize));
    }

    if (backgroundImage.isNull())
    {
        juce::File f("C:/Coding/Tunings VST3/HARBINGER OF DEATH/mAIN bACKGROUND iMAGE/Background.jpg");
        if (f.existsAsFile())
            backgroundImage = juce::ImageFileFormat::loadFrom(f);
    }
}

void HarbingerAudioProcessorEditor::timerCallback()
{
    // 1. Sync Chop LED brightness from DSP LFO
    const float chopBrightness = audioProcessor.getChopLedBrightness();
    const bool chopEngineActive = audioProcessor.isChopEngineActive();
    chopFootswitch.setLedBrightness(chopEngineActive ? std::max(0.3f, chopBrightness) : 0.0f);
    chopFootswitch.setLedActive(chopEngineActive);

    // 2. Sync Trash LED state from State Machine
    trashFootswitch.setLedActive(audioProcessor.isTrashEngineActive());

    // 3. Keep 3-way switches in sync if automated/changed via preset
    const int currentOctMode = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("octaveMode")->load());
    if (octaveModeSwitch.getSelectedIndex() != currentOctMode)
        octaveModeSwitch.setSelectedIndex(currentOctMode, juce::dontSendNotification);

    const int currentTrashMode = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("trashMode")->load());
    if (trashModeSwitch.getSelectedIndex() != currentTrashMode)
        trashModeSwitch.setSelectedIndex(currentTrashMode, juce::dontSendNotification);

    // 4. Sync Power / Enable Button
    const bool isEnabled = audioProcessor.isPluginEnabled();
    if (powerButton.getToggleState() != isEnabled)
    {
        powerButton.setToggleState(isEnabled, juce::dontSendNotification);
        powerButton.setButtonText(isEnabled ? "POWER ON" : "BYPASS");
        repaint();
    }

    // 5. Sync Speed knob label, readout, and slider position with BPM Sync state and host tempo
    const bool isSync = speedSyncToggle.getToggleState();
    const double currentBpm = audioProcessor.getHostBpm();
    if (isSync != lastBpmSyncState || std::abs(currentBpm - lastKnownBpm) > 0.05)
    {
        lastBpmSyncState = isSync;
        lastKnownBpm = currentBpm;
        speedLabel.setText(isSync ? "SPEED [SYNC]" : "SPEED", juce::dontSendNotification);
        speedLabel.setColour(juce::Label::textColourId, isSync ? juce::Colour(0xffff2233) : juce::Colours::white);
        speedSlider.updateText();
    }

    if (isSync)
    {
        const int divIdx = syncDivisionBox.getSelectedItemIndex();
        if (divIdx >= 0 && divIdx <= 10)
        {
            const double targetVal = speedSlider.getMinimum() + (static_cast<double>(divIdx) / 10.0) * (speedSlider.getMaximum() - speedSlider.getMinimum());
            if (std::abs(speedSlider.getValue() - targetVal) > 0.8 && !speedSlider.isMouseButtonDown())
            {
                speedSlider.setValue(targetVal, juce::dontSendNotification);
                speedSlider.updateText();
            }
        }
    }
}

void HarbingerAudioProcessorEditor::drawHexScrew(juce::Graphics& g, float cx, float cy)
{
    const float radius = 7.0f;
    juce::Path hex;
    for (int i = 0; i < 6; ++i)
    {
        const float a = static_cast<float>(i) * (juce::MathConstants<float>::twoPi / 6.0f);
        const float px = cx + radius * std::cos(a);
        const float py = cy + radius * std::sin(a);
        if (i == 0) hex.startNewSubPath(px, py);
        else hex.lineTo(px, py);
    }
    hex.closeSubPath();

    g.setColour(juce::Colour(0xff090a0d));
    g.fillPath(hex);
    g.setColour(juce::Colour(0xff4a5260));
    g.strokePath(hex, juce::PathStrokeType(1.2f));

    // Inner hex depression
    juce::Path inner;
    for (int i = 0; i < 6; ++i)
    {
        const float a = static_cast<float>(i) * (juce::MathConstants<float>::twoPi / 6.0f);
        const float px = cx + (radius * 0.55f) * std::cos(a);
        const float py = cy + (radius * 0.55f) * std::sin(a);
        if (i == 0) inner.startNewSubPath(px, py);
        else inner.lineTo(px, py);
    }
    inner.closeSubPath();
    g.setColour(juce::Colour(0xff181b22));
    g.fillPath(inner);
}

void HarbingerAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Base Obsidian Background
    g.fillAll(juce::Colour(0xff090a0d));

    // 1. Draw Main Background Image covering the ENTIRE plugin window (Image 1 style)
    if (backgroundImage.isValid())
    {
        g.drawImage(backgroundImage, getLocalBounds().toFloat(),
                    juce::RectanglePlacement::stretchToFit | juce::RectanglePlacement::fillDestination);
    }

    // 2. Sleek Dark Modern Top Navigation Bar (matching Image 1)
    const int barH = 54;
    const int w = getWidth();

    // Dark semi-transparent studio header overlay
    g.setColour(juce::Colour(0xee08090d));
    g.fillRect(0, 0, w, barH);

    // Glowing Blood-Red Laser Accent Line under top bar
    g.setColour(juce::Colour(0xff8a0a14));
    g.fillRect(0, barH - 1, w, 1);
    g.setColour(juce::Colour(0x44ff1e2e));
    g.fillRect(0, barH, w, 1);

    // If Bypassed, draw frosted dark overlay with indicator badge
    if (!audioProcessor.isPluginEnabled())
    {
        auto bypassArea = getLocalBounds().withTrimmedTop(barH);
        g.setColour(juce::Colour(0xaa060709));
        g.fillRect(bypassArea);

        auto badge = juce::Rectangle<float>(bypassArea.getCentreX() - 160.0f, bypassArea.getCentreY() - 36.0f, 320.0f, 72.0f);
        g.setColour(juce::Colour(0xf2180407));
        g.fillRoundedRectangle(badge, 8.0f);
        g.setColour(juce::Colour(0xffff2233));
        g.drawRoundedRectangle(badge, 8.0f, 1.5f);

        g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText("PEDAL BYPASSED", badge.removeFromTop(40.0f), juce::Justification::centred);
        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xffff6677));
        g.drawText("CLEAN UNALTERED SIGNAL", badge, juce::Justification::centred);
    }
}

void HarbingerAudioProcessorEditor::resized()
{
    // ==========================================
    // 1. TOP BAR LAYOUT (y = 0 to 54, matching Image 1)
    // ==========================================
    // Left: Preset navigation
    const int navY = 13;
    prevPresetButton.setBounds(16, navY, 24, 28);
    categoryBox.setBounds(44, navY, 135, 28);
    presetBox.setBounds(184, navY, 170, 28);
    nextPresetButton.setBounds(358, navY, 24, 28);
    savePresetButton.setBounds(390, navY, 50, 28);
    deletePresetButton.setBounds(445, navY, 45, 28);

    // Center-Right: Mini Rotary Knobs (INPUT, MIX, OUTPUT)
    inputGainSlider.setBounds(520, 2, 48, 36);
    inputGainLabel.setBounds(520, 37, 48, 14);

    mixSlider.setBounds(580, 2, 48, 36);
    mixLabel.setBounds(580, 37, 48, 14);

    outputGainSlider.setBounds(640, 2, 48, 36);
    outputGainLabel.setBounds(640, 37, 48, 14);

    // Right: Oversampling, MIDI MAP, POWER ON
    oversampleLabel.setBounds(720, 3, 70, 14);
    oversampleBox.setBounds(720, 18, 70, 26);

    midiMenuButton.setBounds(815, navY, 90, 28);
    powerButton.setBounds(920, navY, 105, 28);

    // Sub-Banner (right below top bar)
    bannerLabel.setBounds(0, 56, getWidth(), 20);

    // ==========================================
    // 2. MAIN CONTROLS (Floating directly on Background.jpg)
    // ==========================================
    // Row 1: 4 Main Knobs
    const int knobY = 82;
    const int knobW = 140;
    const int knobH = 155;
    const int labelY = 240;
    const int labelH = 18;

    // Dissonance
    dissonanceSlider.setBounds(110, knobY, knobW, knobH);
    dissonanceLabel.setBounds(110, labelY, knobW, labelH);

    // Square Wave
    squareWaveSlider.setBounds(345, knobY, knobW, knobH);
    squareWaveLabel.setBounds(345, labelY, knobW, labelH);

    // Octave
    octaveSlider.setBounds(595, knobY, knobW, knobH);
    octaveLabel.setBounds(595, labelY, knobW, labelH);

    // Speed
    speedSlider.setBounds(830, knobY, knobW, knobH);
    speedLabel.setBounds(830, labelY, knobW, labelH);

    // Row 2: Mode Selectors & BPM Sync Controls
    // Octave Mode Switch (under Dissonance / left side)
    octaveModeLabel.setBounds(100, 268, 200, 18);
    octaveModeSwitch.setBounds(100, 288, 200, 30);

    // Trash Mode Switch (under Square Wave)
    trashModeLabel.setBounds(335, 268, 220, 18);
    trashModeSwitch.setBounds(335, 288, 220, 30);

    // Chop BPM Sync Controls (under Speed / right side)
    speedSyncToggle.setBounds(840, 266, 120, 22);
    syncDivisionBox.setBounds(840, 290, 120, 26);

    // Row 3: Heavy-Duty Stomp Footswitches (Bottom Left & Bottom Right)
    // Framing "HARBINGER OF DEATH BY THE EDGE OF FEAR" in the center!
    const int stompY = 510;
    const int stompW = 210;
    const int stompH = 195;

    chopFootswitch.setBounds(150, stompY, stompW, stompH);
    trashFootswitch.setBounds(720, stompY, stompW, stompH);

    // Stomp Link Switch (centered between Chop and Trash, below the logo text)
    stompLinkToggle.setBounds(495, 638, 90, 24);

    // Modal overlay if open
    if (midiModal != nullptr)
        midiModal->setBounds(getLocalBounds().reduced(80, 50));
}

void HarbingerAudioProcessorEditor::populateCategories()
{
    syncPresetUIFromProcessorState();
}

void HarbingerAudioProcessorEditor::syncPresetUIFromProcessorState()
{
    categories = audioProcessor.getPresetManager().getCategories();

    categoryBox.clear(juce::dontSendNotification);
    for (size_t i = 0; i < categories.size(); ++i)
        categoryBox.addItem(categories[i], static_cast<int>(i + 1));

    const auto& allPresets = audioProcessor.getPresetManager().getAllPresets();
    int activeGlobalIdx = audioProcessor.getSelectedPresetIndex();
    if (activeGlobalIdx < 0 || activeGlobalIdx >= static_cast<int>(allPresets.size()))
        activeGlobalIdx = 0;

    const auto& currentPreset = allPresets[static_cast<size_t>(activeGlobalIdx)];

    int targetCatIdx = 0;
    for (size_t i = 0; i < categories.size(); ++i)
    {
        if (categories[i] == currentPreset.category)
        {
            targetCatIdx = static_cast<int>(i);
            break;
        }
    }

    categoryBox.setSelectedId(targetCatIdx + 1, juce::dontSendNotification);

    filteredIndices.clear();
    presetBox.clear(juce::dontSendNotification);
    int selectedPresetBoxId = 1;
    int id = 1;
    for (int i = 0; i < static_cast<int>(allPresets.size()); ++i)
    {
        if (allPresets[static_cast<size_t>(i)].category == currentPreset.category)
        {
            if (i == activeGlobalIdx)
                selectedPresetBoxId = id;

            filteredIndices.push_back(i);
            presetBox.addItem(allPresets[static_cast<size_t>(i)].name, id++);
        }
    }

    presetBox.setSelectedId(selectedPresetBoxId, juce::dontSendNotification);
}

void HarbingerAudioProcessorEditor::updatePresetDropdown()
{
    const int catIdx = categoryBox.getSelectedItemIndex();
    if (catIdx < 0 || catIdx >= static_cast<int>(categories.size()))
        return;

    const std::string& catName = categories[static_cast<size_t>(catIdx)];
    const auto& allPresets = audioProcessor.getPresetManager().getAllPresets();

    filteredIndices.clear();
    presetBox.clear(juce::dontSendNotification);

    int id = 1;
    for (int i = 0; i < static_cast<int>(allPresets.size()); ++i)
    {
        if (allPresets[static_cast<size_t>(i)].category == catName)
        {
            filteredIndices.push_back(i);
            presetBox.addItem(allPresets[static_cast<size_t>(i)].name, id++);
        }
    }

    if (presetBox.getNumItems() > 0)
        presetBox.setSelectedId(1, juce::sendNotification);
}

void HarbingerAudioProcessorEditor::comboBoxChanged(juce::ComboBox* box)
{
    if (box == &categoryBox)
    {
        updatePresetDropdown();
    }
    else if (box == &presetBox)
    {
        const int pIdx = presetBox.getSelectedItemIndex();
        if (pIdx >= 0 && pIdx < static_cast<int>(filteredIndices.size()))
        {
            const int globalIndex = filteredIndices[static_cast<size_t>(pIdx)];
            audioProcessor.applyPreset(globalIndex);

            // Sync switches
            octaveModeSwitch.setSelectedIndex(static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("octaveMode")->load()), juce::dontSendNotification);
            trashModeSwitch.setSelectedIndex(static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("trashMode")->load()), juce::dontSendNotification);
        }
    }
}

void HarbingerAudioProcessorEditor::buttonClicked(juce::Button* b)
{
    if (b == &prevPresetButton)
    {
        selectPreviousPreset();
    }
    else if (b == &nextPresetButton)
    {
        selectNextPreset();
    }
    else if (b == &savePresetButton)
    {
        promptSaveUserPreset();
    }
    else if (b == &deletePresetButton)
    {
        const int activeGlobalIdx = audioProcessor.getSelectedPresetIndex();
        if (audioProcessor.getPresetManager().isCurrentPresetFactory(activeGlobalIdx))
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Protected Preset",
                "Factory presets cannot be deleted.",
                "OK");
        }
        else
        {
            audioProcessor.getPresetManager().deleteUserPreset(activeGlobalIdx);
            populateCategories();
        }
    }
    else if (b == &midiMenuButton)
    {
        if (midiModal == nullptr)
        {
            midiModal = std::make_unique<MidiMappingModal>(audioProcessor.getMidiManager(), [this]() {
                midiModal.reset();
                repaint();
            });
            midiModal->setBounds(getLocalBounds().reduced(80, 50));
            addAndMakeVisible(midiModal.get());
        }
        else
        {
            midiModal.reset();
            repaint();
        }
    }
}

void HarbingerAudioProcessorEditor::selectPreviousPreset()
{
    int currentId = presetBox.getSelectedId();
    if (currentId > 1)
        presetBox.setSelectedId(currentId - 1, juce::sendNotification);
    else if (categoryBox.getSelectedId() > 1)
    {
        categoryBox.setSelectedId(categoryBox.getSelectedId() - 1, juce::sendNotification);
        presetBox.setSelectedId(presetBox.getNumItems(), juce::sendNotification);
    }
}

void HarbingerAudioProcessorEditor::selectNextPreset()
{
    int currentId = presetBox.getSelectedId();
    if (currentId < presetBox.getNumItems())
        presetBox.setSelectedId(currentId + 1, juce::sendNotification);
    else if (categoryBox.getSelectedId() < categoryBox.getNumItems())
    {
        categoryBox.setSelectedId(categoryBox.getSelectedId() + 1, juce::sendNotification);
        presetBox.setSelectedId(1, juce::sendNotification);
    }
}

void HarbingerAudioProcessorEditor::promptSaveUserPreset()
{
    auto* aw = new juce::AlertWindow("Save Custom Harbinger Preset", "Enter a name for your custom preset:", juce::AlertWindow::NoIcon);
    aw->addTextEditor("presetName", "My Brutal Deathcore Tone");
    aw->addButton("Save", 1);
    aw->addButton("Cancel", 0);

    aw->enterModalState(true, juce::ModalCallbackFunction::create([this, aw](int result) {
        if (result == 1)
        {
            const auto name = aw->getTextEditorContents("presetName").toStdString();
            if (!name.empty())
            {
                auto& apvts = audioProcessor.getAPVTS();
                HarbingerPresets::Preset p;
                p.dissonance   = apvts.getRawParameterValue("dissonance")->load();
                p.squareWave   = apvts.getRawParameterValue("squareWave")->load();
                p.octaveLevel  = apvts.getRawParameterValue("octaveLevel")->load();
                p.octaveMode   = static_cast<int>(apvts.getRawParameterValue("octaveMode")->load());
                p.speedHz      = apvts.getRawParameterValue("speedHz")->load();
                p.speedBpmSync = apvts.getRawParameterValue("speedBpmSync")->load() > 0.5f;
                p.syncDivision = static_cast<int>(apvts.getRawParameterValue("syncDivision")->load());
                p.trashMode    = static_cast<int>(apvts.getRawParameterValue("trashMode")->load());
                p.trashActive  = audioProcessor.isTrashEngineActive();
                p.mix          = apvts.getRawParameterValue("mix")->load();
                p.outputGain   = apvts.getRawParameterValue("outputGain")->load();
                p.description  = "User created Harbinger of Death preset";

                int newIdx = 0;
                const bool success = audioProcessor.getPresetManager().saveUserPreset(name, p, &newIdx);

                if (success)
                {
                    populateCategories();
                    for (int i = 0; i < categoryBox.getNumItems(); ++i)
                    {
                        if (categoryBox.getItemText(i) == "User Presets")
                        {
                            categoryBox.setSelectedId(i + 1, juce::sendNotification);
                            break;
                        }
                    }
                }
                else
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Preset Error",
                        "Could not overwrite a factory preset. Choose a different name.",
                        "OK");
                }
            }
        }
        delete aw;
    }));
}

void HarbingerAudioProcessorEditor::openMidiContextMenu(const std::string& paramId, const std::string& dispName)
{
    juce::PopupMenu menu;
    menu.addSectionHeader(dispName);

    menu.addItem(1, "Reset Parameter", true);
    menu.addSeparator();
    menu.addItem(2, "Enable MIDI Learn", true, audioProcessor.getMidiManager().getIsLearning() && audioProcessor.getMidiManager().getLearningParamId() == paramId);
    menu.addItem(3, "Open MIDI Mappings...", true);
    menu.addItem(4, "Unmap MIDI", true);

    menu.showMenuAsync(juce::PopupMenu::Options(), [this, paramId, dispName](int result) {
        if (result == 1)
        {
            if (auto* param = audioProcessor.getAPVTS().getParameter(paramId))
                param->setValueNotifyingHost(param->getDefaultValue());
        }
        else if (result == 2)
        {
            audioProcessor.getMidiManager().setLearningParameter(paramId, dispName);
            repaint();
        }
        else if (result == 3)
        {
            if (midiModal == nullptr)
            {
                midiModal = std::make_unique<MidiMappingModal>(audioProcessor.getMidiManager(), [this]() {
                    midiModal.reset();
                    repaint();
                });
                midiModal->setBounds(getLocalBounds().reduced(80, 50));
                addAndMakeVisible(midiModal.get());
            }
        }
        else if (result == 4)
        {
            auto& list = audioProcessor.getMidiManager().getMappings();
            for (int i = static_cast<int>(list.size()) - 1; i >= 0; --i)
            {
                if (list[static_cast<size_t>(i)].parameterId == paramId)
                    audioProcessor.getMidiManager().removeMapping(i);
            }
            if (paramId == "chopStomp")
                audioProcessor.handleChopPointerState(false);
            repaint();
        }
    });
}
