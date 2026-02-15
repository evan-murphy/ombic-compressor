#!/usr/bin/env bash
# Build Ombic Compressor from repo root. Usage:
#   ./scripts/build.sh        → build VST3 (and install to ~/Library on macOS)
#   ./scripts/build.sh dist    → build VST3 + Standalone, then create zip for another machine
# Run from anywhere; the script switches to the repo root.

set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [ ! -f build/CMakeCache.txt ]; then
  echo "Configuring (first time)..."
  cmake -B build -S .
fi

if [ "${1:-}" = "dist" ]; then
  echo "Building distribution zip (VST3 + Standalone)..."
  cmake --build build --target OmbicCompressor_dist
  echo "Done: build/OmbicCompressor-1.0.0-Darwin.zip"
else
  echo "Building VST3..."
  cmake --build build --target OmbicCompressor_VST3
  echo "Done: build/Plugin/OmbicCompressor_artefacts/VST3/"
fi
