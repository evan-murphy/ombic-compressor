# OMBIC Compressor + Neon Saturator VST3
# Requires JUCE 7+ (add_subdirectory(JUCE) or find_package(JUCE) from parent).

cmake_minimum_required(VERSION 3.22)
project(OmbicCompressor VERSION 1.0.0 LANGUAGES CXX)

# JUCE is expected to be available (e.g. add_subdirectory from parent)
if(NOT TARGET juce::juce_recommended_config_flags)
    find_package(JUCE 7.0 CONFIG REQUIRED)
endif()

# Logo and other assets (embedded for header branding)
juce_add_binary_data(OmbicAssets
    HEADER_NAME OmbicAssets.h
    NAMESPACE OmbicAssets
    SOURCES
        "${CMAKE_SOURCE_DIR}/Ombic_Alpha.png"
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

target_sources(OmbicCompressor
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Components/OmbicLookAndFeel.cpp
        Source/Components/CompressorSection.cpp
        Source/Components/SaturatorSection.cpp
        Source/Components/OutputSection.cpp
        Source/Components/MeterStrip.cpp
        Source/Components/TransferCurveComponent.cpp
        Source/Emulation/DataLoader.cpp
        Source/Emulation/MeasuredCompressor.cpp
        Source/Emulation/FRCharacter.cpp
        Source/Emulation/THDCharacter.cpp
        Source/Emulation/NeonTapeSaturation.cpp
        Source/Emulation/MVPChain.cpp
)

target_compile_definitions(OmbicCompressor
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
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

# Bundle curve data into VST3, then re-copy the full bundle to the plugin folder
# (JUCE's copy runs before this, so the installed bundle would lack CurveData otherwise)
set(OMBIC_VST3_BUNDLE "${CMAKE_CURRENT_BINARY_DIR}/OmbicCompressor_artefacts/VST3/Ombic Compressor.vst3")
set(OMBIC_VST3_RESOURCES "${OMBIC_VST3_BUNDLE}/Contents/Resources/CurveData")
add_custom_command(TARGET OmbicCompressor_VST3 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OMBIC_VST3_RESOURCES}/fetish_v2"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OMBIC_VST3_RESOURCES}/lala_v2"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/output/fetish_v2" "${OMBIC_VST3_RESOURCES}/fetish_v2"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/output/lala_v2" "${OMBIC_VST3_RESOURCES}/lala_v2"
  COMMENT "Copying curve data into VST3 bundle"
)
if(APPLE)
  add_custom_command(TARGET OmbicCompressor_VST3 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${OMBIC_VST3_BUNDLE}" "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3"
    COMMENT "Re-copying VST3 bundle (with curve data) to user plugin folder"
  )
endif()
