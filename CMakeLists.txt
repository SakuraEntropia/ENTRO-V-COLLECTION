cmake_minimum_required(VERSION 3.22)

project(AnalogTriad VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
endif()

option(TRIAD_BUILD_TESTS "Build the deterministic engine test executable" ON)
option(TRIAD_UNIVERSAL "Build macOS universal binaries" OFF)
set(TRIAD_JUCE_PATH "" CACHE PATH "Path to a JUCE 9.0.4 checkout")

# ---------------------------------------------------------------------------
# JUCE dependency: kept outside the source tree, pinned through TRIAD_JUCE_PATH.
# ---------------------------------------------------------------------------
if (NOT TRIAD_JUCE_PATH)
    set(_triad_default_juce "${CMAKE_CURRENT_SOURCE_DIR}/../work/JUCE")
    if (EXISTS "${_triad_default_juce}/CMakeLists.txt")
        set(TRIAD_JUCE_PATH "${_triad_default_juce}")
    endif()
endif()

if (NOT TRIAD_JUCE_PATH OR NOT EXISTS "${TRIAD_JUCE_PATH}/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE 9.0.4 was not found. Configure with -DTRIAD_JUCE_PATH=/path/to/JUCE-9.0.4")
endif()

if (TRIAD_UNIVERSAL AND APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
endif()

add_subdirectory("${TRIAD_JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE" EXCLUDE_FROM_ALL)

# ---------------------------------------------------------------------------
# Shared, framework-independent code.
# ---------------------------------------------------------------------------
add_library(triad_core STATIC
    src/core/Dsp.cpp
    src/core/Sequencer.cpp
)
target_include_directories(triad_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_compile_features(triad_core PUBLIC cxx_std_20)

add_library(triad_engines STATIC
    src/engines/Matriarch.cpp
    src/engines/PolyBrute.cpp
    src/engines/One.cpp
    src/engines/Factory.cpp
)
target_link_libraries(triad_engines PUBLIC triad_core)
target_compile_features(triad_engines PUBLIC cxx_std_20)

# ---------------------------------------------------------------------------
# User interface (shared widgets and one panel per instrument).
# ---------------------------------------------------------------------------
add_library(triad_ui STATIC
    src/ui/LookAndFeel.cpp
    src/ui/Widgets.cpp
    src/ui/PanelBase.cpp
    src/ui/PresetBrowser.cpp
    src/ui/KeyboardPanel.cpp
    src/ui/MatriarchPanel.cpp
    src/ui/PolyBrutePanel.cpp
    src/ui/OnePanel.cpp
    src/ui/Editor.cpp
    src/plugin/Presets.cpp
)
target_link_libraries(triad_ui
    PUBLIC
        triad_engines
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_extra
)
target_compile_definitions(triad_ui PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_REPORT_APP_USAGE=0
)

# ---------------------------------------------------------------------------
# Three independent plugin targets built from one host adapter.
# ---------------------------------------------------------------------------
if (APPLE)
    set(TRIAD_PLUGIN_FORMATS AU VST3 Standalone)
else()
    set(TRIAD_PLUGIN_FORMATS VST3 Standalone)
endif()

function(triad_add_instrument target product_name plugin_code instrument_kind)
    juce_add_plugin(${target}
        COMPANY_NAME "SakuraEntropia"
        BUNDLE_ID "com.sakuraentropia.${target}"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS TRUE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE Tria
        PLUGIN_CODE ${plugin_code}
        PLUGIN_NAME "${product_name}"
        PRODUCT_NAME "${product_name}"
        DESCRIPTION "Independent virtual-analog instrument inspired by ${product_name}"
        FORMATS ${TRIAD_PLUGIN_FORMATS}
        VST3_CATEGORIES "Instrument Synth"
        AU_MAIN_TYPE "kAudioUnitType_MusicDevice"
        NEEDS_WEB_BROWSER FALSE
        NEEDS_CURL FALSE
    )

    target_sources(${target}
        PRIVATE
            src/plugin/PluginProcessor.cpp
            src/plugin/PluginEntry.cpp
    )

    target_compile_definitions(${target}
        PUBLIC
            TRIAD_INSTRUMENT_KIND=${instrument_kind}
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_VST3_CAN_REPLACE_VST2=0
                    JUCE_REPORT_APP_USAGE=0
    )

    target_link_libraries(${target}
        PRIVATE
            triad_ui
            triad_engines
            triad_core
            juce::juce_audio_utils
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_gui_extra
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )

    if (APPLE)
        set_target_properties(${target} PROPERTIES
            XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
            XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
    endif()
endfunction()

triad_add_instrument(Matriarch "MATRIARCH V" Ma01 0)
triad_add_instrument(PolyBrute "POLYBRUTE V" Pb02 1)
triad_add_instrument(MoogOne "ONE V" Mo03 2)

# ---------------------------------------------------------------------------
# Deterministic, framework-independent tests.
# ---------------------------------------------------------------------------
if (TRIAD_BUILD_TESTS)
    enable_testing()
    add_executable(triad_tests tests/EngineTests.cpp)
    target_link_libraries(triad_tests PRIVATE triad_engines triad_core)
    target_compile_features(triad_tests PRIVATE cxx_std_20)
    add_test(NAME triad_engine_contracts COMMAND triad_tests)

    add_executable(triad_preset_tests tests/PresetTests.cpp src/plugin/Presets.cpp)
    target_include_directories(triad_preset_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(triad_preset_tests PRIVATE triad_engines juce::juce_core)
    target_compile_features(triad_preset_tests PRIVATE cxx_std_20)
    add_test(NAME triad_factory_presets COMMAND triad_preset_tests)
endif()
