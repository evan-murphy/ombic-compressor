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

Curve data is bundled with the plugin automatically. If you cloned without it, see **docs/ARCHITECTURE.md** and run `./scripts/import-curve-data.sh /path/to/curve-data-for-compressor.tar` if needed.

---

## Signal flow and audio path

**Order of processing:** Input → **Neon saturator** → **Compressor** (Opto or FET) → **Output** (makeup/trim). A **sidechain** is derived from the input in parallel and only feeds the compressor's detector (and an optional Listen monitor).

| Block | What it does |
|-------|----------------|
| **Sidechain filter** | Mono sum of input, optional high-pass (20–500 Hz). At 20 Hz the filter is off (full-range detector). When the HPF is on, only the filtered signal drives gain reduction — e.g. high-pass to keep kick from pumping the compressor. **SC Listen** replaces the main output with the sidechain so you hear what the detector hears. |
| **Neon saturator** | Always in the path before the compressor. Drive, Tone, Mix (dry/wet), Intensity, Burstiness, G Min, Soft Sat After. The neon character is time-varying level (wobble), not just clipping. |
| **Compressor** | **Mode**: Opto (smooth, program-dependent) or FET (faster, ratio/attack/release). **Threshold**, **Ratio**, **Attack**, **Release** (FET only). **Compress / Limit** (Opto only) switches detector flavour. Gain reduction is applied to the main signal; the detector can use the raw main signal, the sidechain (if SC filter is on), or in Opto mode internal LPF/shelf when no external sidechain is used. |
| **Output** | Makeup or trim, -24 dB to +12 dB. If **SC Listen** is on, the output is replaced by the sidechain (no makeup). |

**Shared vs mode-specific:** Sidechain, Neon, and Output are shared. The compressor block is either FET or Opto; only one runs. Opto has fixed envelope and optional internal sidechain (rolloff + Limit shelf) when you're not using the SC filter; FET uses the same sidechain or main signal and respects ratio/attack/release.

---

## GUI

![Ombic Compressor](docs/screenshot.png)

- **Header**: OMBIC branding; “Curve data: OK” when measured data is loaded.
- **SC Filter**: Frequency knob (20 Hz = off), Listen button, frequency response display. (Layout follows the signal path above.)
- **Compressor**: Mode (Opto / FET), threshold, ratio, attack, release, Compress/Limit (Opto), GR meter.
- **Transfer curve**: In vs out (dB) with current operating point.
- **Saturator**: Drive, Tone, Mix, Intensity, Burstiness, G Min, Soft Sat After.
- **Output**: Makeup/trim gain.
- **Meters**: Input, gain reduction, output.

See **docs/GUI_SPEC.md** for the full parameter list and **docs/ARCHITECTURE.md** for how curve data and the two systems work. Testing: **docs/TESTING_PROTOCOL.md**. Automated validation: **ombic-vst-inspector** → `docs/RUNBOOK_OMBIC_VALIDATION.md`.

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
