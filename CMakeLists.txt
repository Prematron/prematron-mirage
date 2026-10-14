cmake_minimum_required(VERSION 3.22)
project(PrematronMirage VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE)
FetchContent_Declare(clap-juce-extensions
    GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
    GIT_TAG main
    GIT_SUBMODULES_RECURSE TRUE)
FetchContent_MakeAvailable(JUCE clap-juce-extensions)

juce_add_plugin(Mirage
    COMPANY_NAME "PREMATRON"
    BUNDLE_ID com.prematron.mirage
    PRODUCT_NAME "Mirage"
    PLUGIN_MANUFACTURER_CODE Prem
    PLUGIN_CODE Mirg
    FORMATS VST3 AU Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES Fx Delay
    AU_MAIN_TYPE kAudioUnitType_Effect
    COPY_PLUGIN_AFTER_BUILD FALSE)

clap_juce_extensions_plugin(TARGET Mirage
    CLAP_ID "com.prematron.mirage"
    CLAP_FEATURES audio-effect delay stereo)

juce_add_binary_data(MirageAssets SOURCES Assets/Prematron_Logo.png)

target_sources(Mirage PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/Parameters.h
    Source/DSP/DelayLine.h
    Source/DSP/SVF.h
    Source/DSP/Diffusion.h
    Source/DSP/Utility.h)

target_compile_definitions(Mirage PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(Mirage
    PRIVATE MirageAssets
            juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)
