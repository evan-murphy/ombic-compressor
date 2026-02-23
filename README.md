# Ombic Compressor

VST3 compressor plugin with **four topologies** (Opto, FET, PWM, VCA), **neon bulb saturation**, optional **Iron** transformer colour, transfer-curve metering, and sidechain filtering. Built with JUCE 7+. **Standalone** — no dependency on any other repo or tool. Curve data is **included in this repo** and packaged with the plugin. For the big picture (curve data, two systems), see **docs/ARCHITECTURE.md**.

The saturator is called “neon bulb” because the level **wobbles randomly in time**, like an old neon sign, not because of how the waveform is clipped — so you get living, unstable level on top of harmonic saturation.

---

## Quickstart (new to the project?)

1. **Clone and enter the repo**
   ```bash
   git clone <this-repo-url> ombic-compressor && cd ombic-compressor
   ```

2. **Build**
   ```bash
   ./scripts/build.sh
   ```
   Requires **CMake 3.22+** and **JUCE 7** (JUCE is fetched automatically). First run configures CMake; later runs just build.

3. **Find the plugin**
   - VST3: `build/Plugin/OmbicCompressor_artefacts/VST3/`  
   - On macOS the build also installs to `~/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3`.

4. **Use it**  
   Rescan VST3s in your DAW (or restart the DAW). Load **Ombic Compressor** on a track. Choose a **compressor mode** (Opto / FET / PWM / VCA), set threshold (and ratio/attack/release or speed depending on mode), then adjust **Neon** and **Output** as needed.

**Curve data:** The repo includes `output/fetish_v2` and `output/lala_v2` (required for Opto/FET) and optionally `output/dbcomp_vca` (for VCA). The build bundles them into the VST3. If you cloned without curve data, see **docs/ARCHITECTURE.md** and run `./scripts/import-curve-data.sh /path/to/curve-data-for-compressor.tar` if you have a tarball.

---

## Modules and signal flow

**Order of processing:** Input → **Neon saturator** → **Compressor** (one of Opto / FET / PWM / VCA) → **Iron** (optional) → **Output** (makeup/trim). A **sidechain** is derived from the input and feeds the compressor’s detector (and an optional Listen monitor).

| Module | What it does |
|--------|----------------|
| **Sidechain filter** | Mono sum of input, optional high-pass (20–500 Hz). At 20 Hz the filter is off (full-range detector). Use the HPF to keep e.g. kick from pumping the compressor. **SC Listen** replaces the main output with the sidechain so you hear what the detector hears. |
| **Neon saturator** | Always in the path before the compressor. **Drive**, **Tone**, **Mix** (dry/wet), **Intensity**, **Burstiness**, **G Min**, **Soft Sat After**. The neon character is time-varying level (wobble), not just clipping. Can be bypassed (Neon On/Off). |
| **Compressor** | One of four modes (see below). Threshold, ratio, and mode-specific controls. Gain reduction is applied to the main signal; the detector can use the main signal, the sidechain (when SC filter is on), or in Opto mode internal LPF/shelf. |
| **Iron** | Optional transformer-style saturation and LF/HF shaping after the compressor. Single **Iron** amount control (0–100). Mode-aware (Opto/FET/PWM). |
| **Output** | Makeup or trim, -24 dB to +12 dB. **Auto Gain** applies an estimated makeup based on threshold/ratio/speed. If **SC Listen** is on, the output is replaced by the sidechain (no makeup). |

---

## Compressor modes

| Mode | Controls | Curve data | Character |
|------|----------|------------|-----------|
| **Opto** | Threshold (0–100%), **Compress / Limit** (detector flavour). No ratio/attack/release. | `lala_v2` (required) | Smooth, program-dependent. |
| **FET** | Threshold (dB), **Ratio**, **Attack**, **Release**. **FET Character** (Off / Rev A / LN) for curve flavour. | `fetish_v2` (required) | Fast, 1176-style; full control. |
| **PWM** | **Threshold**, **Ratio**, **Speed** (single envelope control). No attack/release knobs. | None | Algorithmic, feedback detector; no curve data. |
| **VCA** | **Threshold**, **Ratio** only. | `dbcomp_vca` (optional; DBX 160–style) | Measured-curve, bus-style; feedforward. |

- **Opto** and **FET** require curve data in the repo (`output/lala_v2`, `output/fetish_v2`); the build always bundles them.
- **VCA** is available when `output/dbcomp_vca` exists; the build bundles it when present. If missing, VCA mode does no compression (Neon and output gain still work).
- **PWM** needs no curve data and works even when Opto/FET data is missing (useful for dev builds).

See **docs/VCA_VS_PWM_MODES.md** for PWM vs VCA design, and **docs/NEON_AND_VCA_FAQ.md** for VCA and Neon tips.

---

## Build (reference)

From the repo root:

```bash
./scripts/build.sh
```

- **VST3 only:** `cmake --build build --target OmbicCompressor_VST3`
- **Distribution zip** (VST3 + Standalone for another Mac): `./scripts/build.sh dist` → `build/OmbicCompressor-1.0.0-Darwin.zip`

Curve data is bundled automatically: `output/fetish_v2` and `output/lala_v2` always; `output/dbcomp_vca` when present. Runtime: plugin loads from the VST3 bundle, or from project `output/` / `OMBIC_COMPRESSOR_DATA_PATH` when running from the project directory.

---

## GUI

![Ombic Compressor](docs/screenshot.png)

- **Header**: OMBIC branding; “Curve data: OK” when measured data is loaded.
- **SC Filter**: Frequency (20 Hz = off), Listen, frequency response display.
- **Compressor**: Mode (Opto / FET / PWM / VCA), threshold, and mode-specific controls (ratio, attack, release, speed, Compress/Limit, FET Character). GR meter.
- **Transfer curve**: In vs out (dB) with current operating point (mode-aware).
- **Saturator (Neon)**: Drive, Tone, Mix, Intensity, Burstiness, G Min, Soft Sat After, On/Off.
- **Output**: Makeup/trim gain, **Iron** amount, **Auto Gain** toggle.
- **Meters**: Input, gain reduction, output. **Peak / VU** toggle; peak mode shows L/R and hold.

See **docs/GUI_SPEC.md** for the full parameter list. Testing: **docs/TESTING_PROTOCOL.md**. Automated validation: **ombic-vst-inspector** → `docs/RUNBOOK_OMBIC_VALIDATION.md`.

---

## Distribution / install on another machine

```bash
./scripts/build.sh dist
```

This creates `build/OmbicCompressor-1.0.0-Darwin.zip`. On the other machine: unzip, then copy `VST3/Ombic Compressor.vst3` to `~/Library/Audio/Plug-Ins/VST3/` (and optionally `Standalone/Ombic Compressor.app` to `/Applications`).

You can also copy `build/Plugin/OmbicCompressor_artefacts` and move the VST3 bundle and/or app from there. No separate installer is required; the plugin is self-contained and ships with curve data inside the VST3.

---

## License

MIT (see LICENSE).
