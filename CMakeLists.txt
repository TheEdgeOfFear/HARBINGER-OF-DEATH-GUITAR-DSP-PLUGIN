cmake_minimum_required(VERSION 3.22)

# If built standalone without parent project:
if(NOT TARGET juce::juce_audio_basics)
    project(HarbingerOfDeath VERSION 1.0.0 LANGUAGES C CXX)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.4
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(HarbingerOfDeath
    COMPANY_NAME "THE EDGE OF FEAR"
    BUNDLE_ID com.theedgeoffear.harbingerofdeath
    PLUGIN_MANUFACTURER_CODE TEOF
    PLUGIN_CODE Hbrg
    FORMATS VST3 Standalone
    PRODUCT_NAME "HARBINGER OF DEATH"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(HarbingerOfDeath
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/HarbingerLookAndFeel.h
        Source/HarbingerPresets.h
        Source/PresetManager.h
        Source/MidiManager.h
        Source/MidiMappingModal.h
        Source/DSP/OctaveEngine.h
        Source/DSP/DetuneVoice.h
        Source/DSP/SquareWaveshaper.h
        Source/DSP/ChopModulator.h
        Source/DSP/HarbingerDSPChain.h
)

juce_add_binary_data(Harbinger_Assets
    SOURCES
        "Top Bar Image/HARBINGER_TOP_BAR.jpg"
        "mAIN bACKGROUND iMAGE/Background.jpg"
)

target_link_libraries(HarbingerOfDeath
    PRIVATE
        Harbinger_Assets
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(HarbingerOfDeath
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_ASIO=1
)

target_include_directories(HarbingerOfDeath
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../asiosdk/common"
)

if(MSVC)
    target_compile_options(HarbingerOfDeath PUBLIC /utf-8)
endif()


