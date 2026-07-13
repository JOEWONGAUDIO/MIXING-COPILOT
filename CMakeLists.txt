cmake_minimum_required(VERSION 3.22)

project(MIXING_COPILOT VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (DEFINED JUCE_PATH)
    add_subdirectory(${JUCE_PATH} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        8.0.8
    )
    FetchContent_MakeAvailable(JUCE)
endif()

set(MIXING_COPILOT_FORMATS VST3)

if (APPLE)
    list(APPEND MIXING_COPILOT_FORMATS AU)
endif()

if (DEFINED AAX_SDK_PATH AND EXISTS "${AAX_SDK_PATH}")
    juce_set_aax_sdk_path("${AAX_SDK_PATH}")
    list(APPEND MIXING_COPILOT_FORMATS AAX)
endif()

juce_add_plugin(MixingCopilot
    COMPANY_NAME "JOEWONGAUDIO"
    BUNDLE_ID "com.joewongaudio.mixingcopilot"
    PLUGIN_MANUFACTURER_CODE Jowa
    PLUGIN_CODE MxCp
    FORMATS ${MIXING_COPILOT_FORMATS}
    PRODUCT_NAME "MIXING COPILOT"
    COPY_PLUGIN_AFTER_BUILD TRUE
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
)

juce_generate_juce_header(MixingCopilot)

juce_add_binary_data(MixingCopilotAssets
    SOURCES
        Assets/vintage_metronome.png
        Assets/metronome_pendulum.png
        Assets/mixing_copilot_logo.png
)

target_sources(MixingCopilot
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(MixingCopilot
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(MixingCopilot
    PRIVATE
        juce::juce_audio_utils
        MixingCopilotAssets
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
