#pragma once

#include "HarbingerPresets.h"
#include <juce_core/juce_core.h>
#include <vector>
#include <algorithm>

class PresetManager
{
public:
    PresetManager()
    {
        loadFactoryPresets();
        loadUserPresets();
    }

    const std::vector<HarbingerPresets::Preset>& getAllPresets() const
    {
        return allPresets;
    }

    std::vector<std::string> getCategories() const
    {
        std::vector<std::string> categories;
        for (const auto& p : allPresets)
        {
            if (std::find(categories.begin(), categories.end(), p.category) == categories.end())
                categories.push_back(p.category);
        }
        return categories;
    }

    bool isCurrentPresetFactory(int globalIndex) const
    {
        if (globalIndex >= 0 && globalIndex < static_cast<int>(allPresets.size()))
            return allPresets[static_cast<size_t>(globalIndex)].isFactory;
        return true;
    }

    bool saveUserPreset(const std::string& name,
                        const HarbingerPresets::Preset& presetData,
                        int* outNewIndex = nullptr)
    {
        for (const auto& p : allPresets)
        {
            if (p.name == name && p.isFactory)
                return false; // Prevent overwriting factory presets
        }

        for (size_t i = 0; i < allPresets.size(); ++i)
        {
            if (allPresets[i].name == name && !allPresets[i].isFactory)
            {
                allPresets[i] = presetData;
                allPresets[i].name = name;
                allPresets[i].category = "User Presets";
                allPresets[i].isFactory = false;
                if (outNewIndex) *outNewIndex = static_cast<int>(i);
                saveUserPresetsToDisk();
                return true;
            }
        }

        HarbingerPresets::Preset newPreset = presetData;
        newPreset.name = name;
        newPreset.category = "User Presets";
        newPreset.isFactory = false;

        allPresets.push_back(newPreset);
        if (outNewIndex) *outNewIndex = static_cast<int>(allPresets.size() - 1);
        saveUserPresetsToDisk();
        return true;
    }

    bool deleteUserPreset(int globalIndex)
    {
        if (globalIndex < 0 || globalIndex >= static_cast<int>(allPresets.size()))
            return false;

        if (allPresets[static_cast<size_t>(globalIndex)].isFactory)
            return false;

        allPresets.erase(allPresets.begin() + globalIndex);
        saveUserPresetsToDisk();
        return true;
    }

private:
    std::vector<HarbingerPresets::Preset> allPresets;

    void loadFactoryPresets()
    {
        allPresets = HarbingerPresets::getBuiltInPresets();
    }

    juce::File getUserPresetFile() const
    {
        auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("TheEdgeOfFear")
                   .getChildFile("HarbingerOfDeath");
        if (!dir.exists())
            dir.createDirectory();
        return dir.getChildFile("UserPresets.xml");
    }

    void loadUserPresets()
    {
        auto file = getUserPresetFile();
        if (!file.existsAsFile()) return;

        auto xml = juce::XmlDocument::parse(file);
        if (xml == nullptr || !xml->hasTagName("UserPresets")) return;

        for (auto* child : xml->getChildIterator())
        {
            if (child->hasTagName("Preset"))
            {
                HarbingerPresets::Preset p;
                p.category = "User Presets";
                p.name = child->getStringAttribute("name").toStdString();
                p.description = child->getStringAttribute("description").toStdString();
                p.dissonance = static_cast<float>(child->getDoubleAttribute("dissonance", 0.0));
                p.squareWave = static_cast<float>(child->getDoubleAttribute("squareWave", 0.5));
                p.octaveLevel = static_cast<float>(child->getDoubleAttribute("octaveLevel", 0.5));
                p.octaveMode = child->getIntAttribute("octaveMode", 0);
                p.speedHz = static_cast<float>(child->getDoubleAttribute("speedHz", 8.0));
                p.speedBpmSync = child->getBoolAttribute("speedBpmSync", false);
                p.syncDivision = child->getIntAttribute("syncDivision", 6);
                p.trashMode = child->getIntAttribute("trashMode", 0);
                p.trashActive = child->getBoolAttribute("trashActive", true);
                p.mix = static_cast<float>(child->getDoubleAttribute("mix", 1.0));
                p.outputGain = static_cast<float>(child->getDoubleAttribute("outputGain", 1.0));
                p.isFactory = false;

                allPresets.push_back(p);
            }
        }
    }

    void saveUserPresetsToDisk()
    {
        juce::XmlElement xml("UserPresets");
        for (const auto& p : allPresets)
        {
            if (!p.isFactory)
            {
                auto* child = xml.createNewChildElement("Preset");
                child->setAttribute("name", juce::String(p.name));
                child->setAttribute("description", juce::String(p.description));
                child->setAttribute("dissonance", p.dissonance);
                child->setAttribute("squareWave", p.squareWave);
                child->setAttribute("octaveLevel", p.octaveLevel);
                child->setAttribute("octaveMode", p.octaveMode);
                child->setAttribute("speedHz", p.speedHz);
                child->setAttribute("speedBpmSync", p.speedBpmSync);
                child->setAttribute("syncDivision", p.syncDivision);
                child->setAttribute("trashMode", p.trashMode);
                child->setAttribute("trashActive", p.trashActive);
                child->setAttribute("mix", p.mix);
                child->setAttribute("outputGain", p.outputGain);
            }
        }
        xml.writeTo(getUserPresetFile());
    }
};
