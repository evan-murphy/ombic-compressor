# OMBIC VST — Root CMake (builds Plugin with JUCE)
# From repo root: mkdir build && cd build && cmake .. && cmake --build .
# Requires: CMake 3.22+, Xcode or Ninja.

cmake_minimum_required(VERSION 3.22)
project(OmbicVST VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

# Option 1: JUCE via FetchContent (no local JUCE clone)
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
# Pinned for reproducible builds. JUCE 8.0.11+ includes ScreenCaptureKit fixes for macOS 15;
# bump explicitly when you need new framework features.
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.11
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Plugin target (Source/ + Source/Emulation/ in Plugin/CMakeLists.txt)
add_subdirectory(Plugin)

# Distribution: zip of plugin artefacts for installing on another machine
set(OMBIC_ARTEFACTS_DIR "${CMAKE_BINARY_DIR}/Plugin/OmbicCompressor_artefacts")
set(OMBIC_DIST_ZIP "${CMAKE_BINARY_DIR}/OmbicCompressor-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}.zip")
set(OMBIC_DIST_STAGE "${CMAKE_BINARY_DIR}/_dist_stage")
set(OMBIC_DIST_DEP_LIST OmbicCompressor_VST3 OmbicCompressor_Standalone)
if(APPLE)
  list(APPEND OMBIC_DIST_DEP_LIST OmbicCompressor_AU)
  set(OMBIC_DIST_STAGE_AU_COMMAND "${CMAKE_COMMAND}" -E copy_directory "${OMBIC_ARTEFACTS_DIR}/AU" "${OMBIC_DIST_STAGE}/AU")
else()
  set(OMBIC_DIST_STAGE_AU_COMMAND "${CMAKE_COMMAND}" -E echo "skip: no AU on this platform")
endif()
add_custom_target(OmbicCompressor_dist
  COMMAND ${CMAKE_COMMAND} -E echo "Creating ${OMBIC_DIST_ZIP}"
  COMMAND ${CMAKE_COMMAND} -E remove -f "${OMBIC_DIST_ZIP}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${OMBIC_DIST_STAGE}"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${OMBIC_ARTEFACTS_DIR}/VST3" "${OMBIC_DIST_STAGE}/VST3"
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${OMBIC_ARTEFACTS_DIR}/Standalone" "${OMBIC_DIST_STAGE}/Standalone"
  COMMAND ${OMBIC_DIST_STAGE_AU_COMMAND}
  COMMAND ${CMAKE_COMMAND} -E chdir "${OMBIC_DIST_STAGE}" ${CMAKE_COMMAND} -E tar cvf "${OMBIC_DIST_ZIP}" --format=zip -- .
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${OMBIC_DIST_STAGE}"
  COMMENT "Package Ombic Compressor for distribution (VST3 + Standalone; AU on macOS)"
  DEPENDS ${OMBIC_DIST_DEP_LIST}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
