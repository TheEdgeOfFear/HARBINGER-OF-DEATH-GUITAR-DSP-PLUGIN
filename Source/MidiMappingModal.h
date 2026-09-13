#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "MidiManager.h"

class MidiMappingModal : public juce::Component
{
public:
    MidiMappingModal(MidiManager& manager, std::function<void()> onCloseCallback)
        : midiManager(manager), onClose(onCloseCallback)
    {
        closeButton.setButtonText("X");
        closeButton.onClick = [this]() {
            if (onClose) onClose();
        };
        addAndMakeVisible(closeButton);

        addButton.setButtonText("+ ADD MAPPING");
        addButton.onClick = [this]() {
            MidiMapping newMap;
            newMap.type = MidiMappingType::CC_Absolute;
            newMap.parameterId = "dissonance";
            newMap.displayName = "Dissonance";
            newMap.channel = 0; // Omni
            newMap.controlNumber = 1;
            midiManager.addMapping(newMap);
            rebuildRows();
        };
        addAndMakeVisible(addButton);

        rebuildRows();
    }

    void rebuildRows()
    {
        rows.clear();
        const auto& list = midiManager.getMappings();

        for (int i = 0; i < static_cast<int>(list.size()); ++i)
        {
            auto* row = rows.add(new RowComponent(midiManager, i, [this]() {
                rebuildRows();
            }));
            addAndMakeVisible(row);
        }

        resized();
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        // Dark translucent overlay with red border
        g.setColour(juce::Colour(0xf2101216));
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);
        g.setColour(juce::Colour(0xff800a14));
        g.drawRoundedRectangle(getLocalBounds().toFloat(), 8.0f, 1.5f);

        // Header Title
        g.setFont(juce::FontOptions(15.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText("HARBINGER OF DEATH - MIDI MAPPINGS", 20, 12, getWidth() - 40, 24, juce::Justification::centred);

        // Table Column Labels (aligned precisely with row components)
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff8c92a0));
        g.drawText("Type", 28, 42, 140, 20, juce::Justification::centredLeft);
        g.drawText("Target Parameter", 176, 42, 160, 20, juce::Justification::centredLeft);
        g.drawText("Channel", 344, 42, 65, 20, juce::Justification::centredLeft);
        g.drawText("Control / Note #", 418, 42, 110, 20, juce::Justification::centredLeft);
    }

    void resized() override
    {
        closeButton.setBounds(getWidth() - 32, 10, 22, 22);

        int y = 68;
        const int rowH = 34;

        for (auto* row : rows)
        {
            row->setBounds(12, y, getWidth() - 24, rowH);
            y += rowH + 4;
        }

        addButton.setBounds(20, y + 8, 140, 28);
    }

private:
    class RowComponent : public juce::Component
    {
    public:
        RowComponent(MidiManager& mgr, int index, std::function<void()> onUpdate)
            : manager(mgr), rowIndex(index), notifyUpdate(onUpdate)
        {
            const auto& m = manager.getMappings()[static_cast<size_t>(index)];

            // Type ComboBox
            typeBox.addItem("CC Absolute", 1);
            typeBox.addItem("CC Relative", 2);
            typeBox.addItem("CC Toggle", 3);
            typeBox.addItem("CC Gate", 4);
            typeBox.addItem("CC Preset", 5);
            typeBox.addItem("Note Toggle", 6);
            typeBox.addItem("Note Gate", 7);
            typeBox.addItem("Note Preset", 8);
            typeBox.addItem("Program Change", 9);

            int typeId = 1;
            switch (m.type)
            {
                case MidiMappingType::CC_Absolute:    typeId = 1; break;
                case MidiMappingType::CC_Relative:    typeId = 2; break;
                case MidiMappingType::CC_Toggle:      typeId = 3; break;
                case MidiMappingType::CC_Gate:        typeId = 4; break;
                case MidiMappingType::CC_Preset:      typeId = 5; break;
                case MidiMappingType::Note_Toggle:    typeId = 6; break;
                case MidiMappingType::Note_Gate:      typeId = 7; break;
                case MidiMappingType::Note_Preset:    typeId = 8; break;
                case MidiMappingType::Program_Change: typeId = 9; break;
            }
            typeBox.setSelectedId(typeId, juce::dontSendNotification);
            typeBox.onChange = [this]() {
                auto& item = manager.getMappings()[static_cast<size_t>(rowIndex)];
                switch (typeBox.getSelectedId())
                {
                    case 1: item.type = MidiMappingType::CC_Absolute; break;
                    case 2: item.type = MidiMappingType::CC_Relative; break;
                    case 3: item.type = MidiMappingType::CC_Toggle; break;
                    case 4: item.type = MidiMappingType::CC_Gate; break;
                    case 5: item.type = MidiMappingType::CC_Preset; break;
                    case 6: item.type = MidiMappingType::Note_Toggle; break;
                    case 7: item.type = MidiMappingType::Note_Gate; break;
                    case 8: item.type = MidiMappingType::Note_Preset; break;
                    case 9: item.type = MidiMappingType::Program_Change; break;
                }
                manager.publishSnapshot();
            };
            addAndMakeVisible(typeBox);

            // Parameter Target ComboBox
            paramBox.addItem("Dissonance", 1);
            paramBox.addItem("Square Wave", 2);
            paramBox.addItem("Octave Level", 3);
            paramBox.addItem("Octave Mode", 4);
            paramBox.addItem("Chop Speed", 5);
            paramBox.addItem("Chop Stomp", 6);
            paramBox.addItem("Trash Stomp", 7);
            paramBox.addItem("Trash Mode", 8);
            paramBox.addItem("Speed BPM Sync", 9);
            paramBox.addItem("Sync Division", 10);
            paramBox.addItem("Input Gain", 11);
            paramBox.addItem("Mix", 12);
            paramBox.addItem("Output Gain", 13);
            paramBox.addItem("Link Stomps", 14);
            paramBox.addItem("Plugin Enable", 15);

            if (m.parameterId == "dissonance")       paramBox.setSelectedId(1, juce::dontSendNotification);
            else if (m.parameterId == "squareWave")   paramBox.setSelectedId(2, juce::dontSendNotification);
            else if (m.parameterId == "octaveLevel")  paramBox.setSelectedId(3, juce::dontSendNotification);
            else if (m.parameterId == "octaveMode")   paramBox.setSelectedId(4, juce::dontSendNotification);
            else if (m.parameterId == "speedHz")      paramBox.setSelectedId(5, juce::dontSendNotification);
            else if (m.parameterId == "chopStomp")    paramBox.setSelectedId(6, juce::dontSendNotification);
            else if (m.parameterId == "trashStomp")   paramBox.setSelectedId(7, juce::dontSendNotification);
            else if (m.parameterId == "trashMode")    paramBox.setSelectedId(8, juce::dontSendNotification);
            else if (m.parameterId == "speedBpmSync") paramBox.setSelectedId(9, juce::dontSendNotification);
            else if (m.parameterId == "syncDivision") paramBox.setSelectedId(10, juce::dontSendNotification);
            else if (m.parameterId == "inputGain")    paramBox.setSelectedId(11, juce::dontSendNotification);
            else if (m.parameterId == "mix")          paramBox.setSelectedId(12, juce::dontSendNotification);
            else if (m.parameterId == "outputGain")   paramBox.setSelectedId(13, juce::dontSendNotification);
            else if (m.parameterId == "linkStomps")   paramBox.setSelectedId(14, juce::dontSendNotification);
            else if (m.parameterId == "pluginEnabled")paramBox.setSelectedId(15, juce::dontSendNotification);
            else paramBox.setSelectedId(1, juce::dontSendNotification);

            paramBox.onChange = [this]() {
                auto& item = manager.getMappings()[static_cast<size_t>(rowIndex)];
                switch (paramBox.getSelectedId())
                {
                    case 1:  item.parameterId = "dissonance";    item.displayName = "Dissonance";     break;
                    case 2:  item.parameterId = "squareWave";    item.displayName = "Square Wave";    break;
                    case 3:  item.parameterId = "octaveLevel";   item.displayName = "Octave Level";   break;
                    case 4:  item.parameterId = "octaveMode";    item.displayName = "Octave Mode";    break;
                    case 5:  item.parameterId = "speedHz";       item.displayName = "Chop Speed";     break;
                    case 6:  item.parameterId = "chopStomp";     item.displayName = "Chop Stomp";     break;
                    case 7:  item.parameterId = "trashStomp";    item.displayName = "Trash Stomp";    break;
                    case 8:  item.parameterId = "trashMode";     item.displayName = "Trash Mode";     break;
                    case 9:  item.parameterId = "speedBpmSync";  item.displayName = "Speed BPM Sync"; break;
                    case 10: item.parameterId = "syncDivision";  item.displayName = "Sync Division";  break;
                    case 11: item.parameterId = "inputGain";     item.displayName = "Input Gain";     break;
                    case 12: item.parameterId = "mix";           item.displayName = "Mix";            break;
                    case 13: item.parameterId = "outputGain";    item.displayName = "Output Gain";    break;
                    case 14: item.parameterId = "linkStomps";    item.displayName = "Link Stomps";    break;
                    case 15: item.parameterId = "pluginEnabled"; item.displayName = "Plugin Enable";  break;
                }
                manager.publishSnapshot();
            };
            addAndMakeVisible(paramBox);

            // Channel ComboBox (Omni = 0, 1-16)
            channelBox.addItem("Omni", 1);
            for (int c = 1; c <= 16; ++c)
                channelBox.addItem(juce::String(c), c + 1);

            channelBox.setSelectedId(m.channel == 0 ? 1 : m.channel + 1, juce::dontSendNotification);
            channelBox.onChange = [this]() {
                auto& item = manager.getMappings()[static_cast<size_t>(rowIndex)];
                item.channel = channelBox.getSelectedId() - 1;
                manager.publishSnapshot();
            };
            addAndMakeVisible(channelBox);

            // Control number ComboBox (CC #0 to CC #127, identical to HAMMER)
            for (int cc = 0; cc <= 127; ++cc)
                ccBox.addItem("CC #" + juce::String(cc), cc + 1);

            ccBox.setSelectedId(m.controlNumber + 1, juce::dontSendNotification);
            ccBox.onChange = [this]() {
                auto& item = manager.getMappings()[static_cast<size_t>(rowIndex)];
                item.controlNumber = ccBox.getSelectedId() - 1;
                manager.publishSnapshot();
            };
            addAndMakeVisible(ccBox);

            // Delete Button
            deleteButton.setButtonText("X");
            deleteButton.onClick = [this]() {
                manager.removeMapping(rowIndex);
                if (notifyUpdate) notifyUpdate();
            };
            addAndMakeVisible(deleteButton);
        }

        void paint(juce::Graphics& g) override
        {
            // Indicator dot matching HAMMER style
            g.setColour(juce::Colours::white);
            g.fillEllipse(10.0f, static_cast<float>(getHeight()) * 0.5f - 3.5f, 7.0f, 7.0f);
        }

        void resized() override
        {
            const int h = getHeight();
            typeBox.setBounds(28, 3, 140, h - 6);
            paramBox.setBounds(176, 3, 160, h - 6);
            channelBox.setBounds(344, 3, 65, h - 6);
            ccBox.setBounds(418, 3, 110, h - 6);
            deleteButton.setBounds(getWidth() - 32, 4, 24, h - 8);
        }

    private:
        MidiManager& manager;
        int rowIndex;
        std::function<void()> notifyUpdate;

        juce::ComboBox typeBox;
        juce::ComboBox paramBox;
        juce::ComboBox channelBox;
        juce::ComboBox ccBox;
        juce::TextButton deleteButton;
    };

    MidiManager& midiManager;
    std::function<void()> onClose;
    juce::TextButton closeButton;
    juce::TextButton addButton;
    juce::OwnedArray<RowComponent> rows;
};

