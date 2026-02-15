# OMBIC Compressor + Neon Bulb Saturator — VST3 Plugin

JUCE 7+ VST3 plugin with compressor mode selection (Opto / FET), **neon bulb saturation** block, and saturator always before compressor. GUI follows the [OMBIC Sound Style Guide](../style/OMBIC_SOUND_STYLE_GUIDE.md). The saturator is called “neon bulb” because the level **wobbles randomly in time**, like an old neon sign, not because of how the waveform is clipped.

## Build

Requires [JUCE 7](https://github.com/juce-framework/JUCE). From the **repo root** (parent of `Plugin/`):

```bash
cmake -B build -S .
cmake --build build --target OmbicCompressor_VST3
```

The VST3 is built under `build/Plugin/OmbicCompressor_artefacts/VST3/` and (on macOS) is copied to `~/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3`. Curve data (`output/fetish_v2`, `output/lala_v2`) is required and is always bundled into the plugin.

To embed in another CMake project that already has JUCE:

```bash
add_subdirectory(path/to/JUCE)
add_subdirectory(path/to/ombic-compressor/Plugin)
```

## GUI

- **Header**: OMBIC red bar, plugin title; “Curve data: OK” when measured data is loaded.
- **Signal flow**: Fixed as IN → Saturator → Compressor → OUT (no order toggle).
- **Transfer curve**: In vs Out (dB) with 1:1 reference; blue curve and red dot for current operating point (mode-aware for Opto vs FET).
- **Compressor section**: Mode (Opto / FET); threshold, ratio, attack, release; gain-reduction meter. Opto shows only threshold; FET shows all.
- **Saturator section**: Drive, Intensity, Tone, Mix (neon bulb saturation; Intensity scales saturation for overblown tones).
- **Output section**: Output gain (makeup/trim), -24…+12 dB (boost capped for safe listening), after saturator and compressor in the signal path.
- **Meter strip**: Input level, gain reduction, output level (updated from processor atomics).

## Parameters (APVTS)

See [GUI_SPEC.md](../docs/GUI_SPEC.md) for full parameter IDs and attachment mapping. All controls use `SliderAttachment`, `ComboBoxAttachment`, or `ButtonAttachment` to `AudioProcessorValueTreeState`.

## DSP

- **Compressor**: FET mode uses threshold (dB), ratio, attack/release with envelope smoothing from `timing.csv`; Opto uses threshold 0–100 with a gentler curve. Curve data is required and is always packaged with the plugin.
- **Curve data**: Required. Lives in this repo at `output/fetish_v2` and `output/lala_v2`; the build always copies them into the VST3’s `Contents/Resources/CurveData/`. At runtime the plugin loads from the bundle (or from `OMBIC_COMPRESSOR_DATA_PATH` if set). On macOS the build also copies the bundle to the user plugin folder.
- **Neon bulb saturator**: The “neon” character comes from **stochastic gain modulation**: gain is driven by filtered noise (and optional burst events), so level varies irregularly. Waveshaping (tanh) is separate; the neon is the modulation, not the curve. Drive and Intensity control depth and how hard the signal is driven; at high Intensity the saturator can be fully overblown. Always in the signal path; Mix is dry/wet.
- **Signal path**: Saturator → compressor (fixed order), then output gain. Implemented by `emulation::MVPChain` (FET or Opto), `emulation::NeonTapeSaturation`, and final output gain in the processor.

## Safe listening

Output gain is limited to +12 dB maximum so that typical nominal levels (e.g. −18 dBFS) stay in a reasonable range when combined with your DAW and monitoring. Use a limiter on your master or headphone bus if you need a hard ceiling (e.g. −1 to −6 dBFS true peak), and keep monitoring levels at or below ~85 dB SPL for extended sessions.
