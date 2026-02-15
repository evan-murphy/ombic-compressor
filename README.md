# Ombic Compressor

VST3 compressor plugin with dual topology (Opto / FET), neon-style saturation, and transfer-curve metering. Built with JUCE 7+.

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
- **Compressor**: Mode (Opto / FET); threshold, ratio, attack, release; gain-reduction meter.
- **Saturator**: Drive, Intensity, Tone, Mix (neon-style saturation).
- **Output**: Makeup/trim gain.
- **Meters**: Input, gain reduction, output.

See `docs/GUI_SPEC.md` for parameters and `docs/TESTING_PROTOCOL.md` for testing in a DAW.

---

## Distribution / install on another machine

The build produces portable artefacts you can copy to another Mac:

- **VST3**: `build/Plugin/OmbicCompressor_artefacts/VST3/Ombic Compressor.vst3` (folder bundle)
- **Standalone app**: `build/Plugin/OmbicCompressor_artefacts/Standalone/Ombic Compressor.app`

**Option A — one zip to transfer**

Build the distribution package (builds VST3 + Standalone first, then zips them):

```bash
cmake --build build --target OmbicCompressor_dist
```

This creates `build/OmbicCompressor-1.0.0-Darwin.zip` containing `VST3/` and `Standalone/`. Copy the zip to the other machine, unzip, then:

- **VST3**: Copy `VST3/Ombic Compressor.vst3` to `~/Library/Audio/Plug-Ins/VST3/` (or `/Library/Audio/Plug-Ins/VST3/` for all users).
- **Standalone** (optional): Copy `Standalone/Ombic Compressor.app` to `/Applications` or leave in the folder and run from there.

**Option B — copy artefacts manually**

Copy the whole `build/Plugin/OmbicCompressor_artefacts` folder to the other machine, then move the VST3 bundle and/or app as above.

No separate installer is required; the plugin is self-contained (curve data is inside the VST3 bundle). For a signed/notarized macOS installer (e.g. .pkg or .dmg) you’d add code signing and use a tool like CPack `productbuild` or create-dmg; the zip is enough for alpha/testing and for your own machines.

---

## License

MIT (see LICENSE).
