# Ombic Compressor

VST3 compressor plugin with dual topology (LALA / FETish), neon-style saturation, and transfer-curve metering. Built with JUCE 7+.

**Curve data** for the compressor is produced by **Ombic VST Inspector** (separate repo). Inspector holds the *raw* measurement data; this repo holds the *finished* `output/fetish_v2` and `output/lala_v2` that get **bundled into the plugin**. To update: run Inspector, then either copy those two folders here or use Inspector’s export script and this repo’s import script (see `docs/WORKFLOW_TWO_REPOS.md`). Run Inspector against your reference plugins, then copy `output/fetish_v2` and `output/lala_v2` into this project (see Build) or set `OMBIC_COMPRESSOR_DATA_PATH` at runtime.

---

## Build

Requires **CMake 3.22+** and **JUCE 7** (fetched automatically). From the repo root:

```bash
cmake -B build -S .
cmake --build build --target OmbicCompressor_VST3
```

The VST3 is built under `build/Plugin/OmbicCompressor_artefacts/VST3/`. On macOS, the build copies curve data from `output/fetish_v2` and `output/lala_v2` into the bundle and installs to `~/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3`.

If you don’t have `output/` yet: in **Ombic VST Inspector** run the ripper (or analyze FETish and LALA), then run `./scripts/export-curve-data-for-compressor.sh` and in *this* repo run `./scripts/import-curve-data.sh /path/to/curve-data-for-compressor.tar`. See `docs/WORKFLOW_TWO_REPOS.md` for how the two repos work together.

---

## GUI

- **Header**: OMBIC branding; “Curve data: OK” when measured data is loaded.
- **Signal flow**: IN → Saturator → Compressor → OUT.
- **Transfer curve**: In vs Out (dB) with current operating point.
- **Compressor**: Mode (LALA / FETish); threshold, ratio, attack, release; gain-reduction meter.
- **Saturator**: Drive, Intensity, Tone, Mix (neon-style saturation).
- **Output**: Makeup/trim gain.
- **Meters**: Input, gain reduction, output.

See `docs/GUI_SPEC.md` for parameters and `docs/TESTING_PROTOCOL.md` for testing in a DAW.

---

## License

MIT (see LICENSE).
