cmake_minimum_required(VERSION 3.22)

project(FOSSPedalboard VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(FOSSPEDALBOARD_BUILD_TESTS "Build the Catch2 unit tests" ON)
option(FOSSPEDALBOARD_WARNINGS_AS_ERRORS "Treat warnings as errors in project sources" ON)

# ---------------------------------------------------------------------------
# JUCE (git submodule, pinned)
# ---------------------------------------------------------------------------
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE submodule is missing. Run:\n"
        "    git submodule update --init --recursive")
endif()
add_subdirectory(external/JUCE)

# ---------------------------------------------------------------------------
# Project sources shared by the plugin and the tests
# ---------------------------------------------------------------------------
set(FOSSPEDALBOARD_SOURCES
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/core/PedalChain.cpp
    src/core/PedalRegistry.cpp
    src/pedals/DelayPedal.cpp
    src/pedals/ReverbPedal.cpp
    src/ui/HandDrawnLookAndFeel.cpp
    src/ui/InfoIcon.cpp
    src/ui/ParameterPanel.cpp
    src/ui/PedalComponent.cpp
    src/ui/SlotComponent.cpp
    src/ui/BoardComponent.cpp
)

set(FOSSPEDALBOARD_HEADERS
    src/PluginProcessor.h
    src/PluginEditor.h
    src/core/Pedal.h
    src/core/PedalChain.h
    src/core/PedalRegistry.h
    src/core/ParameterIds.h
    src/pedals/DelayPedal.h
    src/pedals/ReverbPedal.h
    src/ui/HandDrawnLookAndFeel.h
    src/ui/InfoIcon.h
    src/ui/ParameterPanel.h
    src/ui/PedalComponent.h
    src/ui/SlotComponent.h
    src/ui/BoardComponent.h
)

# Applies the project's compile settings to a JUCE target.
function(fosspedalboard_configure_target target)
    target_sources(${target} PRIVATE ${FOSSPEDALBOARD_SOURCES} ${FOSSPEDALBOARD_HEADERS})
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

    target_compile_definitions(${target} PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_REPORT_APP_USAGE=0
        JUCE_MODAL_LOOPS_PERMITTED=0
    )

    target_link_libraries(${target}
        PRIVATE
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags
    )
endfunction()

# Warnings-as-errors only on our own translation units, never on JUCE's.
if(FOSSPEDALBOARD_WARNINGS_AS_ERRORS)
    if(MSVC)
        set_source_files_properties(${FOSSPEDALBOARD_SOURCES} PROPERTIES COMPILE_OPTIONS "/WX")
    else()
        set_source_files_properties(${FOSSPEDALBOARD_SOURCES} PROPERTIES COMPILE_OPTIONS "-Werror")
    endif()
endif()

# ---------------------------------------------------------------------------
# Plugin
# ---------------------------------------------------------------------------
juce_add_plugin(FOSSPedalboard
    PRODUCT_NAME "FOSS Pedalboard"
    COMPANY_NAME "FOSS Pedalboard"
    BUNDLE_ID com.fosspedalboard.plugin
    PLUGIN_MANUFACTURER_CODE Fspb
    PLUGIN_CODE Fpdb
    FORMATS VST3 Standalone
    VST3_CATEGORIES Fx Delay Reverb
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

fosspedalboard_configure_target(FOSSPedalboard)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(FOSSPEDALBOARD_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.8.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    juce_add_console_app(FOSSPedalboardTests PRODUCT_NAME "FOSS Pedalboard Tests")
    fosspedalboard_configure_target(FOSSPedalboardTests)
    target_sources(FOSSPedalboardTests PRIVATE
        tests/JuceTestInit.cpp
        tests/test_PedalChain.cpp
        tests/test_Pedals.cpp
        tests/test_Processor.cpp
    )
    target_link_libraries(FOSSPedalboardTests PRIVATE Catch2::Catch2WithMain)

    # A single ctest entry rather than catch_discover_tests: discovery runs the
    # binary as a post-build step, which fails on machines where unsigned fresh
    # executables are blocked, and would then fail the whole build.
    enable_testing()
    add_test(NAME FOSSPedalboardTests COMMAND FOSSPedalboardTests --reporter compact)
endif()
