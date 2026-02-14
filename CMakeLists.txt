# OMBIC VST — Root CMake (builds Plugin with JUCE)
# From repo root: mkdir build && cd build && cmake .. && cmake --build .
# Requires: CMake 3.22+, Xcode or Ninja.

cmake_minimum_required(VERSION 3.22)
project(OmbicVST VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

# Option 1: JUCE via FetchContent (no local JUCE clone)
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
# 7.0.9/8.0.0 fail on macOS 15 (CGWindowListCreateImage removed). Use develop for ScreenCaptureKit fix.
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        develop
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# Plugin target (Source/ + Source/Emulation/ in Plugin/CMakeLists.txt)
add_subdirectory(Plugin)
