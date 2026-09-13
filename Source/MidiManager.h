#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <vector>
#include <string>
#include <functional>
#include <atomic>
#include <memory>

enum class MidiMappingType
{
    CC_Absolute,
    CC_Relative,
    CC_Toggle,
    CC_Gate,      // Momentary (>=64 is down, <64 is up)
    CC_Preset,
    Note_Toggle,
    Note_Gate,
    Note_Preset,
    Program_Change
};

inline std::string getMidiMappingTypeName(MidiMappingType t)
{
    switch (t)
    {
        case MidiMappingType::CC_Absolute:    return "CC Absolute";
        case MidiMappingType::CC_Relative:    return "CC Relative";
        case MidiMappingType::CC_Toggle:      return "CC Toggle";
        case MidiMappingType::CC_Gate:        return "CC Gate (Momentary)";
        case MidiMappingType::CC_Preset:      return "CC Preset";
        case MidiMappingType::Note_Toggle:    return "Note Toggle";
        case MidiMappingType::Note_Gate:      return "Note Gate";
        case MidiMappingType::Note_Preset:    return "Note Preset";
        case MidiMappingType::Program_Change: return "Program Change";
        default: return "CC Absolute";
    }
}

struct MidiMapping
{
    MidiMappingType type{MidiMappingType::CC_Absolute};
    std::string parameterId;   // e.g. "dissonance", "chopStomp", "trashStomp"
    std::string displayName;   // e.g. "Dissonance"
    int channel{0};            // 0 = Omni, 1-16
    int controlNumber{11};     // CC number, Note number, or Program number
    float minValue{0.0f};
    float maxValue{1.0f};
};

class MidiManager
{
public:
    MidiManager()
    {
        // Factory default MIDI mappings
        mappings.push_back({MidiMappingType::CC_Absolute, "dissonance",  "Dissonance",       0, 1,  0.0f, 1.0f}); // Mod Wheel CC #1
        mappings.push_back({MidiMappingType::CC_Gate,     "chopStomp",    "Chop Footswitch",  0, 64, 0.0f, 1.0f}); // Sustain Pedal CC #64
        mappings.push_back({MidiMappingType::CC_Toggle,   "trashStomp",   "Trash Footswitch", 0, 65, 0.0f, 1.0f}); // CC #65
        mappings.push_back({MidiMappingType::CC_Absolute, "octaveLevel", "Octave Level",     0, 11, 0.0f, 1.0f}); // Expression CC #11
        publishSnapshot();
    }

    void setLearningParameter(const std::string& paramId, const std::string& dispName)
    {
        learningParamId = paramId;
        learningDisplayName = dispName;
        isLearning.store(true);
    }

    bool getIsLearning() const { return isLearning.load(); }
    std::string getLearningParamId() const { return learningParamId; }

    void cancelLearning()
    {
        isLearning.store(false);
        learningParamId.clear();
        learningDisplayName.clear();
    }

    const std::vector<MidiMapping>& getMappings() const { return mappings; }
    std::vector<MidiMapping>& getMappings() { return mappings; }

    void addMapping(const MidiMapping& m)
    {
        mappings.push_back(m);
        publishSnapshot();
    }

    void removeMapping(int index)
    {
        if (index >= 0 && index < static_cast<int>(mappings.size()))
        {
            mappings.erase(mappings.begin() + index);
            publishSnapshot();
        }
    }

    void updateMapping(int index, const MidiMapping& m)
    {
        if (index >= 0 && index < static_cast<int>(mappings.size()))
        {
            mappings[static_cast<size_t>(index)] = m;
            publishSnapshot();
        }
    }

    void clearMappings()
    {
        mappings.clear();
        publishSnapshot();
    }

    void publishSnapshot()
    {
        auto newSnapshot = std::make_shared<std::vector<MidiMapping>>(mappings);
        activeSnapshot.store(newSnapshot);
    }

    // Process incoming MIDI buffer in audio processing callback (lock-free)
    void processMidi(const juce::MidiBuffer& midiMessages,
                     juce::AudioProcessorValueTreeState& apvts,
                     std::function<void(int presetIndex)> onPresetChange,
                     std::function<void(const std::string& paramId, bool isDown, MidiMappingType type)> onStompAction = nullptr)
    {
        auto snapshot = activeSnapshot.load();
        if (!snapshot && !isLearning.load())
            return;

        for (const auto metadata : midiMessages)
        {
            const auto msg = metadata.getMessage();

            if (msg.isController())
            {
                const int ch = msg.getChannel();
                const int cc = msg.getControllerNumber();
                const int val = msg.getControllerValue();

                if (isLearning.load() && !learningParamId.empty())
                {
                    MidiMapping newMap;
                    newMap.parameterId = learningParamId;
                    newMap.displayName = learningDisplayName;
                    newMap.channel = ch;
                    newMap.controlNumber = cc;

                    if (learningParamId == "chopStomp")
                        newMap.type = MidiMappingType::CC_Gate;
                    else if (learningParamId == "trashStomp" || learningParamId == "speedBpmSync" || learningParamId == "linkStomps" || learningParamId == "pluginEnabled")
                        newMap.type = MidiMappingType::CC_Toggle;
                    else
                        newMap.type = MidiMappingType::CC_Absolute;

                    // Remove prior mapping for this parameter to prevent conflicting duplicate bindings
                    for (int i = static_cast<int>(mappings.size()) - 1; i >= 0; --i)
                    {
                        if (mappings[static_cast<size_t>(i)].parameterId == learningParamId)
                            mappings.erase(mappings.begin() + i);
                    }

                    mappings.push_back(newMap);
                    publishSnapshot();
                    isLearning.store(false);
                    learningParamId.clear();
                    learningDisplayName.clear();
                    continue;
                }

                if (snapshot)
                {
                    for (const auto& m : *snapshot)
                    {
                        if ((m.channel == 0 || m.channel == ch) && m.controlNumber == cc)
                        {
                            const float normalized = static_cast<float>(val) / 127.0f;

                            if (m.type == MidiMappingType::CC_Absolute)
                            {
                                if (auto* param = apvts.getParameter(m.parameterId))
                                    param->setValueNotifyingHost(normalized);

                                if (onStompAction && (m.parameterId == "chopStomp" || m.parameterId == "trashStomp" || m.parameterId == "pluginEnabled"))
                                {
                                    onStompAction(m.parameterId, val >= 64, MidiMappingType::CC_Absolute);
                                }
                            }
                            else if (m.type == MidiMappingType::CC_Toggle)
                            {
                                if (val >= 64)
                                {
                                    if (auto* param = apvts.getParameter(m.parameterId))
                                    {
                                        const float cur = param->getValue();
                                        param->setValueNotifyingHost(cur > 0.5f ? 0.0f : 1.0f);
                                    }
                                    if (onStompAction)
                                        onStompAction(m.parameterId, true, MidiMappingType::CC_Toggle);
                                }
                            }
                            else if (m.type == MidiMappingType::CC_Gate)
                            {
                                const bool isDown = (val >= 64);
                                if (auto* param = apvts.getParameter(m.parameterId))
                                    param->setValueNotifyingHost(isDown ? 1.0f : 0.0f);
                                if (onStompAction)
                                    onStompAction(m.parameterId, isDown, MidiMappingType::CC_Gate);
                            }
                            else if (m.type == MidiMappingType::CC_Preset)
                            {
                                if (onPresetChange)
                                    onPresetChange(val);
                            }
                        }
                    }
                }
            }
            else if (msg.isNoteOnOrOff())
            {
                const int ch = msg.getChannel();
                const int note = msg.getNoteNumber();
                const bool isNoteOn = msg.isNoteOn();

                if (isLearning.load() && !learningParamId.empty() && isNoteOn)
                {
                    MidiMapping newMap;
                    newMap.type = (learningParamId == "chopStomp") ? MidiMappingType::Note_Gate : MidiMappingType::Note_Toggle;
                    newMap.parameterId = learningParamId;
                    newMap.displayName = learningDisplayName;
                    newMap.channel = ch;
                    newMap.controlNumber = note;

                    for (int i = static_cast<int>(mappings.size()) - 1; i >= 0; --i)
                    {
                        if (mappings[static_cast<size_t>(i)].parameterId == learningParamId)
                            mappings.erase(mappings.begin() + i);
                    }

                    mappings.push_back(newMap);
                    publishSnapshot();
                    isLearning.store(false);
                    learningParamId.clear();
                    learningDisplayName.clear();
                    continue;
                }

                if (snapshot)
                {
                    for (const auto& m : *snapshot)
                    {
                        if ((m.channel == 0 || m.channel == ch) && m.controlNumber == note)
                        {
                            if (m.type == MidiMappingType::Note_Gate)
                            {
                                if (auto* param = apvts.getParameter(m.parameterId))
                                    param->setValueNotifyingHost(isNoteOn ? 1.0f : 0.0f);
                                if (onStompAction)
                                    onStompAction(m.parameterId, isNoteOn, MidiMappingType::Note_Gate);
                            }
                            else if (m.type == MidiMappingType::Note_Toggle && isNoteOn)
                            {
                                if (auto* param = apvts.getParameter(m.parameterId))
                                {
                                    const float cur = param->getValue();
                                    param->setValueNotifyingHost(cur > 0.5f ? 0.0f : 1.0f);
                                }
                                if (onStompAction)
                                    onStompAction(m.parameterId, true, MidiMappingType::Note_Toggle);
                            }
                            else if (m.type == MidiMappingType::Note_Preset && isNoteOn)
                            {
                                if (onPresetChange)
                                    onPresetChange(note);
                            }
                        }
                    }
                }
            }
            else if (msg.isProgramChange())
            {
                const int prog = msg.getProgramChangeNumber();
                if (onPresetChange)
                    onPresetChange(prog);
            }
        }
    }

    juce::ValueTree exportToValueTree() const
    {
        juce::ValueTree tree("MidiMappings");
        for (const auto& m : mappings)
        {
            juce::ValueTree item("Mapping");
            item.setProperty("type", static_cast<int>(m.type), nullptr);
            item.setProperty("paramId", juce::String(m.parameterId), nullptr);
            item.setProperty("dispName", juce::String(m.displayName), nullptr);
            item.setProperty("channel", m.channel, nullptr);
            item.setProperty("cc", m.controlNumber, nullptr);
            tree.addChild(item, -1, nullptr);
        }
        return tree;
    }

    void importFromValueTree(const juce::ValueTree& tree)
    {
        if (!tree.hasType("MidiMappings")) return;
        mappings.clear();
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            auto item = tree.getChild(i);
            MidiMapping m;
            m.type = static_cast<MidiMappingType>(static_cast<int>(item.getProperty("type", 0)));
            m.parameterId = item.getProperty("paramId", "").toString().toStdString();
            m.displayName = item.getProperty("dispName", "").toString().toStdString();
            m.channel = item.getProperty("channel", 0);
            m.controlNumber = item.getProperty("cc", item.getProperty("control", 11));
            mappings.push_back(m);
        }
        publishSnapshot();
    }

private:
    std::vector<MidiMapping> mappings;
    std::atomic<std::shared_ptr<std::vector<MidiMapping>>> activeSnapshot;
    std::atomic<bool> isLearning{false};
    std::string learningParamId;
    std::string learningDisplayName;
};

