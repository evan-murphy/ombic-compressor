#!/usr/bin/env bash
# Import curve data tarball (from Ombic VST Inspector export) into this repo's output/.
# Run from Ombic Compressor repo root: ./scripts/import-curve-data.sh /path/to/curve-data-for-compressor.tar

set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARBALL="${1:?Usage: $0 /path/to/curve-data-for-compressor.tar}"

if [ ! -f "$TARBALL" ]; then
  echo "Not found: $TARBALL" >&2
  exit 1
fi

mkdir -p "$ROOT/output"
tar -xvf "$TARBALL" -C "$ROOT/output"
echo "Curve data imported into $ROOT/output/"
echo "Build the plugin to bundle it into the VST3: cmake --build build --target OmbicCompressor_VST3"
