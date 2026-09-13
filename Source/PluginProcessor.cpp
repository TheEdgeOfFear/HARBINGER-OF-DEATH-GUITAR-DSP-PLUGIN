#include "PluginProcessor.h"
#include "PluginEditor.h"

HarbingerAudioProcessor::HarbingerAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
    // Apply default preset
    applyPreset(0);
}

HarbingerAudioProcessor::~HarbingerAudioProcessor() = default;

const juce::String HarbingerAudioProcessor::getName() const
{
    return "HARBINGER OF DEATH";
}

bool HarbingerAudioProcessor::acceptsMidi() const { return true; }
bool HarbingerAudioProcessor::producesMidi() const { return false; }
bool HarbingerAudioProcessor::isMidiEffect() const { return false; }
double HarbingerAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int HarbingerAudioProcessor::getNumPrograms()
{
    return static_cast<int>(presetManager.getAllPresets().size());
}

int HarbingerAudioProcessor::getCurrentProgram()
{
    return currentPresetIndex.load();
}

void HarbingerAudioProcessor::setCurrentProgram(int index)
{
    applyPreset(index);
}

const juce::String HarbingerAudioProcessor::getProgramName(int index)
{
    const auto& all = presetManager.getAllPresets();
    if (index >= 0 && index < static_cast<int>(all.size()))
        return all[static_cast<size_t>(index)].name;
    return {};
}

void HarbingerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void HarbingerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const int numIn = getTotalNumInputChannels();
    dspChain.prepare(sampleRate, samplesPerBlock, std::max(1, numIn));
}

void HarbingerAudioProcessor::releaseResources()
{
    dspChain.reset();
}

bool HarbingerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void HarbingerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // 1. Process MIDI Events (CC, Learn, Footswitches, Program Change)
    midiManager.processMidi(
        midiMessages,
        parameters,
        [this](int presetIndex) { applyPreset(presetIndex); },
        [this](const std::string& paramId, bool isDown, MidiMappingType type) {
            if (paramId == "chopStomp")
            {
                midiChopHeld.store(isDown);
                if (auto* p = parameters.getParameter("chopStomp"))
                    p->setValueNotifyingHost(isDown ? 1.0f : 0.0f);
            }
            else if (paramId == "trashStomp")
            {
                const int mode = static_cast<int>(parameters.getRawParameterValue("trashMode")->load());
                if (mode == 0) // Latching
                {
                    if (type == MidiMappingType::CC_Toggle || type == MidiMappingType::Note_Toggle || isDown)
                    {
                        const bool newState = !trashLatched.load();
                        trashLatched.store(newState);
                        if (auto* p = parameters.getParameter("trashStomp"))
                            p->setValueNotifyingHost(newState ? 1.0f : 0.0f);
                    }
                }
                else // Momentary or Tap+Hold
                {
                    midiTrashHeld.store(isDown);
                    if (auto* p = parameters.getParameter("trashStomp"))
                        p->setValueNotifyingHost(isDown ? 1.0f : 0.0f);
                }
            }
        }
    );

    // 2. Query Host Playhead BPM, PPQ Position & Transport State
    bool isHostPlaying = false;
    double hostBpm = 120.0;
    double hostPpq = -1.0;
    if (auto* pHead = getPlayHead())
    {
        if (auto pos = pHead->getPosition())
        {
            if (pos->getBpm()) hostBpm = *pos->getBpm();
            if (pos->getPpqPosition()) hostPpq = *pos->getPpqPosition();
            isHostPlaying = pos->getIsPlaying();
        }
    }

    currentHostBpm.store(hostBpm);
    hostIsPlaying.store(isHostPlaying);

    // Check Master Plugin Enable / Bypass
    if (!isPluginEnabled())
    {
        chopEngineActive.store(false);
        trashEngineActive.store(false);
        return; // Complete transparent dry bypass
    }

    // 3. Resolve Footswitch State Machine (Chop & Trash)
    const int trashMode = static_cast<int>(parameters.getRawParameterValue("trashMode")->load());
    const bool chopHeld = guiChopHeld.load() || midiChopHeld.load()
                          || (parameters.getRawParameterValue("chopStomp")->load() > 0.5f);
    chopEngineActive.store(chopHeld);

    const bool stompLink = parameters.getRawParameterValue("linkStomps") != nullptr
                           && (parameters.getRawParameterValue("linkStomps")->load() > 0.5f);

    bool trashActive = false;
    if (trashMode == 0) // Latching
    {
        trashActive = trashLatched.load();
    }
    else if (trashMode == 1) // Momentary
    {
        trashActive = trashMomentary.load() || midiTrashHeld.load();
    }
    else if (trashMode == 2) // Tap + Hold
    {
        trashActive = trashLatched.load() || trashMomentary.load() || midiTrashHeld.load();
    }

    // When Link toggle is ON, pressing Chop engages the Trash circuit simultaneously!
    if (stompLink && chopHeld)
        trashActive = true;

    trashEngineActive.store(trashActive);

    // 4. Update Oversampling if changed
    const int oversampleChoice = static_cast<int>(parameters.getRawParameterValue("oversampling")->load());
    const int targetFactor = (oversampleChoice == 0) ? 2 : ((oversampleChoice == 2) ? 8 : 4);
    if (dspChain.getOversamplingFactor() != targetFactor)
        dspChain.setOversamplingFactor(targetFactor);

    // 5. Read APVTS Parameters
    const float inGain       = parameters.getRawParameterValue("inputGain")->load();
    const float dissonance   = parameters.getRawParameterValue("dissonance")->load();
    const float squareWave   = parameters.getRawParameterValue("squareWave")->load();
    const float octaveLevel  = parameters.getRawParameterValue("octaveLevel")->load();
    const int octChoice      = static_cast<int>(parameters.getRawParameterValue("octaveMode")->load());
    const auto octaveMode    = static_cast<HarbingerDSP::OctaveMode>(juce::jlimit(0, 2, octChoice));

    const float speedHz      = parameters.getRawParameterValue("speedHz")->load();
    const bool speedBpmSync  = parameters.getRawParameterValue("speedBpmSync")->load() > 0.5f;
    const int divChoice      = static_cast<int>(parameters.getRawParameterValue("syncDivision")->load());
    const auto syncDiv       = static_cast<HarbingerDSP::SyncDivision>(juce::jlimit(0, 10, divChoice));

    const float mix          = parameters.getRawParameterValue("mix")->load();
    const float outGain      = parameters.getRawParameterValue("outputGain")->load();

    // 6. Process Audio through integrated DSP Pipeline
    dspChain.process(buffer,
                     inGain,
                     dissonance,
                     squareWave,
                     octaveLevel,
                     octaveMode,
                     speedHz,
                     speedBpmSync,
                     syncDiv,
                     trashActive,
                     chopHeld,
                     mix,
                     outGain,
                     hostBpm,
                     hostPpq,
                     isHostPlaying);
}

bool HarbingerAudioProcessor::isPluginEnabled() const
{
    if (auto* param = parameters.getRawParameterValue("pluginEnabled"))
        return param->load() > 0.5f;
    return true;
}

void HarbingerAudioProcessor::handleChopPointerState(bool isDown)
{
    guiChopHeld.store(isDown);
    chopEngineActive.store(isDown || midiChopHeld.load());
    if (auto* param = parameters.getParameter("chopStomp"))
        param->setValueNotifyingHost(isDown ? 1.0f : 0.0f);
}

void HarbingerAudioProcessor::handleTrashPointerState(bool isDown)
{
    const int mode = static_cast<int>(parameters.getRawParameterValue("trashMode")->load());

    if (mode == 1) // Momentary
    {
        trashMomentary.store(isDown);
        trashEngineActive.store(isDown || midiTrashHeld.load());
    }
    else if (mode == 2) // Tap + Hold
    {
        if (isDown)
        {
            trashPressTimestamp = juce::Time::currentTimeMillis();
            trashMomentary.store(true);
            trashEngineActive.store(true);
        }
        else
        {
            const juce::int64 duration = juce::Time::currentTimeMillis() - trashPressTimestamp;
            trashMomentary.store(false);

            if (duration < 300) // Tap (< 300ms) -> Toggle latch
            {
                const bool newLatched = !trashLatched.load();
                trashLatched.store(newLatched);
                trashEngineActive.store(newLatched);
            }
            else // Hold (> 300ms) -> Was momentary, now released
            {
                trashLatched.store(false);
                trashEngineActive.store(false);
            }
        }
    }
    else // Latching mode
    {
        // Latching toggle is handled on click
        juce::ignoreUnused(isDown);
    }

    if (auto* param = parameters.getParameter("trashStomp"))
        param->setValueNotifyingHost(trashEngineActive.load() ? 1.0f : 0.0f);
}

void HarbingerAudioProcessor::handleTrashClick()
{
    const int mode = static_cast<int>(parameters.getRawParameterValue("trashMode")->load());
    if (mode == 0) // Latching
    {
        const bool newLatched = !trashLatched.load();
        trashLatched.store(newLatched);
        trashEngineActive.store(newLatched);
        if (auto* param = parameters.getParameter("trashStomp"))
            param->setValueNotifyingHost(newLatched ? 1.0f : 0.0f);
    }
}

void HarbingerAudioProcessor::applyPreset(int globalIndex)
{
    const auto& all = presetManager.getAllPresets();
    if (globalIndex < 0 || globalIndex >= static_cast<int>(all.size()))
        return;

    currentPresetIndex.store(globalIndex);
    const auto& p = all[static_cast<size_t>(globalIndex)];

    if (auto* param = parameters.getParameter("dissonance"))
        param->setValueNotifyingHost(param->convertTo0to1(p.dissonance));

    if (auto* param = parameters.getParameter("squareWave"))
        param->setValueNotifyingHost(param->convertTo0to1(p.squareWave));

    if (auto* param = parameters.getParameter("octaveLevel"))
        param->setValueNotifyingHost(param->convertTo0to1(p.octaveLevel));

    if (auto* param = parameters.getParameter("octaveMode"))
        param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(p.octaveMode)));

    if (auto* param = parameters.getParameter("speedHz"))
        param->setValueNotifyingHost(param->convertTo0to1(p.speedHz));

    if (auto* param = parameters.getParameter("speedBpmSync"))
        param->setValueNotifyingHost(p.speedBpmSync ? 1.0f : 0.0f);

    if (auto* param = parameters.getParameter("syncDivision"))
        param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(p.syncDivision)));

    if (auto* param = parameters.getParameter("trashMode"))
        param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(p.trashMode)));

    trashLatched.store(p.trashActive);
    if (auto* param = parameters.getParameter("trashStomp"))
        param->setValueNotifyingHost(p.trashActive ? 1.0f : 0.0f);

    if (auto* param = parameters.getParameter("mix"))
        param->setValueNotifyingHost(param->convertTo0to1(p.mix));

    if (auto* param = parameters.getParameter("outputGain"))
        param->setValueNotifyingHost(param->convertTo0to1(p.outputGain));
}

void HarbingerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    state.setProperty("currentPresetIndex", currentPresetIndex.load(), nullptr);
    state.setProperty("trashLatched", trashLatched.load(), nullptr);
    state.addChild(midiManager.exportToValueTree(), -1, nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void HarbingerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
    {
        auto state = juce::ValueTree::fromXml(*xmlState);
        parameters.replaceState(state);

        if (state.hasProperty("currentPresetIndex"))
            currentPresetIndex.store(static_cast<int>(state.getProperty("currentPresetIndex")));

        if (state.hasProperty("trashLatched"))
            trashLatched.store(static_cast<bool>(state.getProperty("trashLatched")));

        auto midiChild = state.getChildWithName("MidiMappings");
        if (midiChild.isValid())
            midiManager.importFromValueTree(midiChild);
        else
            midiManager.clearMappings();
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout HarbingerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // 0. Dissonance (0.0 to 1.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"dissonance", 1},
        "Dissonance",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.15f));

    // 1. Square Wave (0.0 to 1.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"squareWave", 1},
        "Square Wave",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.85f));

    // 2. Octave Level (0.0 to 1.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"octaveLevel", 1},
        "Octave",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.65f));

    // 3. Octave Mode Choice (+1, +2, Both)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"octaveMode", 1},
        "Octave Mode",
        juce::StringArray{ "+1 Octave", "+2 Octaves", "Both (+1 & +2)" },
        2));

    // 4. Chop Speed Hz (0.5 Hz to 35.0 Hz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"speedHz", 1},
        "Chop Speed",
        juce::NormalisableRange<float>(0.5f, 35.0f, 0.01f, 0.45f),
        8.0f));

    // 5. Chop Speed BPM Sync
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"speedBpmSync", 1},
        "Speed BPM Sync",
        false));

    // 6. Sync Division Choice
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"syncDivision", 1},
        "Sync Division",
        juce::StringArray{ "1/4", "1/4 T", "1/4 D", "1/8", "1/8 T", "1/8 D", "1/16", "1/16 T", "1/16 D", "1/32", "1/32 T" },
        6)); // Default 1/16

    // 7. Trash Footswitch Mode Choice (Latching, Momentary, Tap+Hold)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"trashMode", 1},
        "Trash Mode",
        juce::StringArray{ "Latching", "Momentary", "Tap+Hold" },
        0));

    // 8. Trash Stomp State
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"trashStomp", 1},
        "Trash Stomp",
        true));

    // 9. Chop Stomp State
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"chopStomp", 1},
        "Chop Stomp",
        false));

    // 10. Link Footswitches
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"linkStomps", 1},
        "Link Stomps",
        false));

    // 11. Oversampling Mode Choice (2x, 4x, 8x)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"oversampling", 1},
        "Oversampling",
        juce::StringArray{ "2x", "4x", "8x" },
        1)); // Default 4x

    // 11. Input Gain (0.0 to 2.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"inputGain", 1},
        "Input Gain",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f));

    // 12. Master Mix (0.0 to 1.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mix", 1},
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f));

    // 13. Master Output Gain (0.0 to 2.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"outputGain", 1},
        "Output Gain",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f));

    // 14. Master Plugin Enable / Bypass
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"pluginEnabled", 1},
        "Plugin Enabled",
        true));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HarbingerAudioProcessor();
}

juce::AudioProcessorEditor* HarbingerAudioProcessor::createEditor()
{
    return new HarbingerAudioProcessorEditor(*this);
}

bool HarbingerAudioProcessor::hasEditor() const
{
    return true;
}
