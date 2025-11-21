cmake_minimum_required(VERSION 3.15)

project(fdnr-vst3 VERSION 0.0.3)

# Check if JUCE is already available (e.g. if this is a subproject)
if(NOT TARGET juce::juce_audio_utils)
    include(FetchContent)
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.12
    )
    FetchContent_MakeAvailable(juce)
endif()

juce_add_plugin(FDNR
    COMPANY_NAME "OpenSourceCommunity"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTED TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE FdnR
    FORMATS VST3 Standalone
    PRODUCT_NAME "Feedback Delay Network Reverberator"
)

target_sources(FDNR
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/ReverbProcessor.cpp
        Source/ReverbProcessor.h
)

target_compile_features(FDNR PRIVATE cxx_std_17)

target_compile_definitions(FDNR
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA REQUIRED alsa)
endif()

target_link_libraries(FDNR
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        ${ALSA_LIBRARIES}
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

enable_testing()

juce_add_gui_app(ScreenshotTest
    PRODUCT_NAME "ScreenshotTest"
)

target_sources(ScreenshotTest
    PRIVATE
        Tests/ScreenshotTest.cpp
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/ReverbProcessor.cpp
        Source/ReverbProcessor.h
)

target_link_libraries(ScreenshotTest
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
        ${ALSA_LIBRARIES}
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_definitions(ScreenshotTest
    PRIVATE
        JucePlugin_Name="Feedback Delay Network Reverberator"
        JucePlugin_VersionString="0.0.3"
        JucePlugin_WantsMidiInput=0
        JucePlugin_ProducesMidiOutput=0
        JucePlugin_IsMidiEffect=0
        JucePlugin_IsSynth=0
)

target_compile_features(ScreenshotTest PRIVATE cxx_std_17)

add_test(NAME GenerateScreenshot COMMAND ScreenshotTest WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
