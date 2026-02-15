# Ombic Compressor

VST3 compressor plugin with dual topology (Opto / FET), **neon bulb saturation**, and transfer-curve metering. Built with JUCE 7+. **Standalone** — no dependency on any other repo or tool. Curve data is **included in this repo** and **always packaged** with the plugin. For the big picture (curve data, two systems, how they work together), see **docs/ARCHITECTURE.md**.

The saturator is called “neon bulb” because the level **wobbles randomly in time**, like an old neon sign, not because of how the waveform is clipped — so you get living, unstable level on top of harmonic saturation.

---

## Build

Requires **CMake 3.22+** and **JUCE 7** (fetched automatically).

**Easy way** — run from anywhere in the repo:

```bash
./scripts/build.sh
```

First run configures CMake; after that it just builds. The VST3 ends up in `build/Plugin/OmbicCompressor_artefacts/VST3/` and on macOS is also installed to `~/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3`.

Curve data lives in **`output/fetish_v2`** and **`output/lala_v2`** (committed in this repo). The build always bundles it into the VST3. If you cloned without that data, run `./scripts/import-curve-data.sh /path/to/curve-data-for-compressor.tar`, then build again.

---

## GUI

- **Header**: OMBIC branding; “Curve data: OK” when measured data is loaded.
- **Signal flow**: IN → Saturator → Compressor → OUT.
- **Transfer curve**: In vs Out (dB) with current operating point.
- **Compressor**: Mode (Opto / FET); threshold, ratio, attack, release; gain-reduction meter.
- **Saturator**: Drive, Intensity, Tone, Mix (neon bulb saturation).
- **Output**: Makeup/trim gain.
- **Meters**: Input, gain reduction, output.

See `docs/GUI_SPEC.md` for parameters and `docs/TESTING_PROTOCOL.md` for testing in a DAW.

---

## Distribution / install on another machine

To build a **single zip** you can copy to another Mac (VST3 + Standalone app):

```bash
./scripts/build.sh dist
```

This creates `build/OmbicCompressor-1.0.0-Darwin.zip`. On the other machine: unzip, then copy `VST3/Ombic Compressor.vst3` to `~/Library/Audio/Plug-Ins/VST3/` (and optionally `Standalone/Ombic Compressor.app` to `/Applications`).

**Or** copy the folder `build/Plugin/OmbicCompressor_artefacts` to the other machine and move the VST3 bundle and/or app from there.

No separate installer is required; the plugin is self-contained and always ships with curve data inside the VST3. For a signed/notarized macOS installer (e.g. .pkg or .dmg) you’d add code signing and use a tool like CPack `productbuild` or create-dmg; the zip is enough for alpha/testing and for your own machines.

---

## License

MIT (see LICENSE).
