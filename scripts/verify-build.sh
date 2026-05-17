#!/usr/bin/env bash
# Local QA: configure Release, build VST3 (+ AU on macOS), list artefacts.
# Usage: from repo root: ./scripts/verify-build.sh

set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

BUILD_DIR="${BUILD_DIR:-build-verify}"
echo "==> Configuring in ${BUILD_DIR} ..."
cmake -B "${BUILD_DIR}" -S . -DCMAKE_BUILD_TYPE=Release

echo "==> Building OmbicCompressor_VST3 ..."
cmake --build "${BUILD_DIR}" --config Release --target OmbicCompressor_VST3

if [[ "$(uname -s)" == "Darwin" ]]; then
  echo "==> Building OmbicCompressor_AU (Logic) ..."
  cmake --build "${BUILD_DIR}" --config Release --target OmbicCompressor_AU
fi

ART="${BUILD_DIR}/Plugin/OmbicCompressor_artefacts"
echo "==> Artefacts:"
find "${ART}" -maxdepth 4 -type d \( -name '*.vst3' -o -name '*.component' -o -name 'Standalone' \) 2>/dev/null || true
ls -la "${ART}/VST3" 2>/dev/null || true
if [[ "$(uname -s)" == "Darwin" ]]; then
  ls -la "${ART}/AU" 2>/dev/null || true
fi

echo "==> Optional: load AU in Logic or run auval (Audio Unit validation) on the .component when present."
echo "==> Done."
