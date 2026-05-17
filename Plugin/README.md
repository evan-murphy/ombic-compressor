# Ombic Compressor — VST3 Plugin

JUCE 7+ VST3 plugin with compressor mode selection (Opto / FET / PWM / VCA), **neon bulb saturation** block, and saturator always before compressor. The saturator is called “neon bulb” because the level **wobbles randomly in time**, like an old neon sign, not because of how the waveform is clipped.

## Build

Requires [JUCE 7](https://github.com/juce-framework/JUCE). From the **repo root** (parent of `Plugin/`):

```bash
cmake -B build -S .
cmake --build build --target OmbicCompressor_VST3
```

The VST3 is built under `build/Plugin/OmbicCompressor_artefacts/VST3/` and (on macOS) is copied to `~/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3`. Curve data: Opto and FET curve data are required and always bundled; VCA curve data is optional and bundled when present (see **docs/ARCHITECTURE.md** for paths).

To embed in another CMake project that already has JUCE:

```bash
add_subdirectory(path/to/JUCE)
add_subdirectory(path/to/ombic-compressor/Plugin)
```

## GUI

- **Header**: Plugin title; “Curve data: OK” when measured data is loaded.
- **Signal flow**: Fixed as IN → Saturator → Compressor → OUT (no order toggle).
- **Transfer curve**: In vs Out (dB) with 1:1 reference; blue curve and red dot for current operating point (mode-aware for Opto vs FET).
- **Compressor section**: Mode (Opto / FET / PWM / VCA); threshold, ratio, attack, release; gain-reduction meter. Opto shows only threshold; FET shows all.
- **Saturator section**: Drive, Intensity, Tone, Mix (neon bulb saturation; Intensity scales saturation for overblown tones).
- **Output section**: Output gain (makeup/trim), -24…+12 dB (boost capped for safe listening), after saturator and compressor in the signal path.
- **Meter strip**: Input level, gain reduction, output level (updated from processor atomics). Peak/VU toggle; stereo L/R in peak mode.

## Metering

- **Level meters** show **peak** level by default (fast attack, slow release). A thin line indicates a **2 s peak hold** so you can read the maximum. Scale includes **−18 dB** as a common reference level for gain staging.
- **Peak / VU**: Use the strip’s **Peak** and **VU** buttons to switch level display. **Peak** = fast response with L/R stereo bars and hold. **VU** = average (RMS) level with ~300 ms ballistics.
- **GR (gain reduction)** shows how much the compressor is reducing level, with fast response and a short hold. Colour coding (e.g. teal / yellow / red) indicates amount of reduction.
- **Transfer curve** red dot = current **average (RMS)** input vs output level.

## Parameters (APVTS)

See [GUI_SPEC.md](../docs/GUI_SPEC.md) for full parameter IDs and attachment mapping. All controls use `SliderAttachment`, `ComboBoxAttachment`, or `ButtonAttachment` to `AudioProcessorValueTreeState`.

## DSP

- **Compressor**: FET mode uses threshold (dB), ratio, attack/release with envelope smoothing from `timing.csv`; Opto uses threshold 0–100 with a gentler curve. Curve data is required and is always packaged with the plugin.
- **Curve data**: Required for Opto and FET; optional for VCA. Lives in this repo under `output/` (see **docs/ARCHITECTURE.md** for layout); the build copies it (VCA only if present) into the VST3’s `Contents/Resources/CurveData/`. At runtime the plugin loads from the bundle (or from `OMBIC_COMPRESSOR_DATA_PATH` if set). On macOS the build also copies the bundle to the user plugin folder.
- **Neon bulb saturator**: The “neon” character comes from **stochastic gain modulation**: gain is driven by filtered noise (and optional burst events), so level varies irregularly. Waveshaping (tanh) is separate; the neon is the modulation, not the curve. Drive and Intensity control depth and how hard the signal is driven; at high Intensity the saturator can be fully overblown. Always in the signal path; Mix is dry/wet.
- **Signal path**: Saturator → compressor (fixed order), then output gain. Implemented by `emulation::MVPChain` (FET or Opto), `emulation::NeonTapeSaturation`, and final output gain in the processor.

## Safe listening

Output gain is limited to +12 dB maximum so that typical nominal levels (e.g. −18 dBFS) stay in a reasonable range when combined with your DAW and monitoring. Use a limiter on your master or headphone bus if you need a hard ceiling (e.g. −1 to −6 dBFS true peak), and keep monitoring levels at or below ~85 dB SPL for extended sessions.
