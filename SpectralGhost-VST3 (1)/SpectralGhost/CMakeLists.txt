cmake_minimum_required(VERSION 3.22)
project(SpectralGhost VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(SpectralGhost
    COMPANY_NAME             "SpectralGhostLab"
    COMPANY_WEBSITE          ""
    BUNDLE_ID                "com.spectralghostlab.spectralghost"
    PLUGIN_MANUFACTURER_CODE Sgl1
    PLUGIN_CODE              Sgh1
    FORMATS                  VST3
    PRODUCT_NAME             "Spectral Ghost"
    IS_SYNTH                 FALSE
    NEEDS_MIDI_INPUT         FALSE
    NEEDS_MIDI_OUTPUT        FALSE
    IS_MIDI_EFFECT           FALSE
    COPY_PLUGIN_AFTER_BUILD  FALSE
    VST3_CATEGORIES          "Fx" "Modulation" "Spatial")

juce_generate_juce_header(SpectralGhost)

target_sources(SpectralGhost PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/dsp/HPSSplitter.cpp
    Source/dsp/StreamEngines.cpp
    Source/ui/AeroLookAndFeel.cpp)

target_compile_definitions(SpectralGhost PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_MODAL_LOOPS_PERMITTED=1)

target_link_libraries(SpectralGhost PRIVATE
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)
