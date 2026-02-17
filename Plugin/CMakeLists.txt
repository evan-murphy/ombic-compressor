# OMBIC Compressor + Neon Saturator VST3
# Requires JUCE 7+ (add_subdirectory(JUCE) or find_package(JUCE) from parent).

cmake_minimum_required(VERSION 3.22)
project(OmbicCompressor VERSION 1.0.0 LANGUAGES CXX)

# JUCE is expected to be available (e.g. add_subdirectory from parent)
if(NOT TARGET juce::juce_recommended_config_flags)
    find_package(JUCE 7.0 CONFIG REQUIRED)
endif()

# Logo: packaged with the plugin (required)
set(OMBIC_LOGO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Ombic_Alpha.png")
if(NOT EXISTS "${OMBIC_LOGO_FILE}")
    message(FATAL_ERROR "Logo missing: ${OMBIC_LOGO_FILE} — add Ombic_Alpha.png to Plugin/Assets/ to build.")
endif()

# v2 editor: main view as tube, Neon knobs only. Set to OFF to use v1 editor.
option(OMBIC_USE_V2_EDITOR "Use v2 editor (main view as tube)" ON)

# Curve data: required and always packaged with the plugin (no dependency on external tools)
set(OMBIC_CURVE_FETISH "${CMAKE_SOURCE_DIR}/output/fetish_v2")
set(OMBIC_CURVE_LALA "${CMAKE_SOURCE_DIR}/output/lala_v2")
set(OMBIC_CURVE_VCA "${CMAKE_SOURCE_DIR}/output/dbcomp_vca")
if(NOT EXISTS "${OMBIC_CURVE_FETISH}")
    message(FATAL_ERROR "Curve data required: ${OMBIC_CURVE_FETISH} missing. This repo must contain output/fetish_v2 and output/lala_v2.")
endif()
if(NOT EXISTS "${OMBIC_CURVE_LALA}")
    message(FATAL_ERROR "Curve data required: ${OMBIC_CURVE_LALA} missing. This repo must contain output/fetish_v2 and output/lala_v2.")
endif()

# Trash fonts: embed from Plugin/fonts/ when present (design-spec-files/OMBIC_COMPRESSOR_JUCE_SPEC.md)
set(OMBIC_ASSET_SOURCES "${OMBIC_LOGO_FILE}")
set(OMBIC_FONTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fonts")
if(EXISTS "${OMBIC_FONTS_DIR}/Trash-Regular.ttf")
    list(APPEND OMBIC_ASSET_SOURCES "${OMBIC_FONTS_DIR}/Trash-Regular.ttf")
endif()
if(EXISTS "${OMBIC_FONTS_DIR}/Trash-Bold.ttf")
    list(APPEND OMBIC_ASSET_SOURCES "${OMBIC_FONTS_DIR}/Trash-Bold.ttf")
endif()

juce_add_binary_data(OmbicAssets
    HEADER_NAME OmbicAssets.h
    NAMESPACE OmbicAssets
    SOURCES ${OMBIC_ASSET_SOURCES}
)

juce_add_plugin(OmbicCompressor
    COMPANY_NAME "Ombic Sound"
    PLUGIN_MANUFACTURER_CODE Obmc
    PLUGIN_CODE Omc1
    BUNDLE_ID "com.ombicsound.OmbicCompressor"
    FORMATS VST3 Standalone
    PRODUCT_NAME "Ombic Compressor"
    IS_SYNTH FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
)

set(OMBIC_PLUGIN_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Components/OmbicLookAndFeel.cpp
    Source/Components/CompressorSection.cpp
    Source/Components/SaturatorSection.cpp
    Source/Components/OutputSection.cpp
    Source/Components/SidechainFilterSection.cpp
    Source/Components/MeterStrip.cpp
    Source/Components/TransferCurveComponent.cpp
    Source/Components/MainVuComponent.cpp
    Source/Emulation/DataLoader.cpp
)
if(OMBIC_USE_V2_EDITOR)
    list(APPEND OMBIC_PLUGIN_SOURCES
        Source/PluginEditorV2.cpp
        Source/Components/MainViewAsTubeComponent.cpp
    )
endif()
target_sources(OmbicCompressor
    PRIVATE
        ${OMBIC_PLUGIN_SOURCES}
        Source/Emulation/MeasuredCompressor.cpp
        Source/Emulation/FRCharacter.cpp
        Source/Emulation/THDCharacter.cpp
        Source/Emulation/NeonTapeSaturation.cpp
        Source/Emulation/MVPChain.cpp
        Source/Emulation/PwmCompressor.cpp
        Source/Emulation/PwmChain.cpp
        Source/Emulation/IronTransformer.cpp
)

target_compile_definitions(OmbicCompressor
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        $<$<BOOL:${OMBIC_USE_V2_EDITOR}>:OMBIC_USE_V2_EDITOR>
)

target_link_libraries(OmbicCompressor
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        OmbicAssets
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

target_include_directories(OmbicCompressor
    PRIVATE
        Source
        Source/Components
        Source/Emulation
)

juce_generate_juce_header(OmbicCompressor)

# Bundle curve data into VST3 (required; always packaged; VCA optional)
set(OMBIC_VST3_BUNDLE "${CMAKE_CURRENT_BINARY_DIR}/OmbicCompressor_artefacts/VST3/Ombic Compressor.vst3")
set(OMBIC_VST3_RESOURCES "${OMBIC_VST3_BUNDLE}/Contents/Resources/CurveData")
add_custom_command(TARGET OmbicCompressor_VST3 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OMBIC_VST3_RESOURCES}/fetish_v2"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OMBIC_VST3_RESOURCES}/lala_v2"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${OMBIC_CURVE_FETISH}" "${OMBIC_VST3_RESOURCES}/fetish_v2"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${OMBIC_CURVE_LALA}" "${OMBIC_VST3_RESOURCES}/lala_v2"
  COMMENT "Packaging curve data into VST3 bundle"
)
if(EXISTS "${OMBIC_CURVE_VCA}/compression_curve.csv")
  add_custom_command(TARGET OmbicCompressor_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OMBIC_VST3_RESOURCES}/dbcomp_vca"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${OMBIC_CURVE_VCA}" "${OMBIC_VST3_RESOURCES}/dbcomp_vca"
    COMMENT "Packaging VCA (dbcomp) curve data into VST3 bundle"
  )
endif()
if(APPLE)
  add_custom_command(TARGET OmbicCompressor_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${OMBIC_VST3_BUNDLE}" "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3"
    COMMENT "Re-copying VST3 bundle (with curve data) to user plugin folder"
  )
endif()
