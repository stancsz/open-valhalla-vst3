cmake_minimum_required(VERSION 3.15)

project(VST3-Open-Valhalla VERSION 0.0.3)

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

juce_add_plugin(VST3OpenValhalla
    COMPANY_NAME "OpenSourceCommunity"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTED TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE VstO
    FORMATS VST3 Standalone
    PRODUCT_NAME "Open Valhalla"
)

target_sources(VST3OpenValhalla
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/ReverbProcessor.cpp
        Source/ReverbProcessor.h
)

target_compile_features(VST3OpenValhalla PRIVATE cxx_std_17)

target_compile_definitions(VST3OpenValhalla
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA REQUIRED alsa)
endif()

target_link_libraries(VST3OpenValhalla
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        ${ALSA_LIBRARIES}
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
