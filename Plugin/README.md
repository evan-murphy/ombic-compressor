# OMBIC Compressor + Neon Saturator — VST3 Plugin

JUCE 7+ VST3 plugin with compressor mode selection (LALA / FETish), neon saturation block, and saturator always before compressor. GUI follows the [OMBIC Sound Style Guide](../style/OMBIC_SOUND_STYLE_GUIDE.md).

## Build

Requires [JUCE 7](https://github.com/juce-framework/JUCE). From the **repo root** (parent of `Plugin/`):

```bash
cmake -B build -S .
cmake --build build --target OmbicCompressor_VST3
```

The VST3 is built under `build/Plugin/OmbicCompressor_artefacts/VST3/` and (on macOS) is copied to `~/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3` after curve data is bundled.

To embed in another CMake project that already has JUCE:

```bash
add_subdirectory(path/to/JUCE)
add_subdirectory(path/to/VST/Plugin)
```

## GUI

- **Header**: OMBIC red bar, plugin title; “Curve data: OK” when measured data is loaded.
- **Signal flow**: Fixed as IN → Saturator → Compressor → OUT (no order toggle).
- **Transfer curve**: In vs Out (dB) with 1:1 reference; blue curve and red dot for current operating point (mode-aware for LALA vs FETish).
- **Compressor section**: Mode (LALA / FETish); threshold, ratio, attack, release; gain-reduction meter. LALA shows only threshold; FETish shows all.
- **Saturator section**: Drive, Intensity, Tone, Mix (neon saturation; Intensity scales saturation for overblown tones).
- **Output section**: Output gain (makeup/trim), -24…+24 dB, after saturator and compressor in the signal path.
- **Meter strip**: Input level, gain reduction, output level (updated from processor atomics).

## Parameters (APVTS)

See [GUI_SPEC.md](../docs/GUI_SPEC.md) for full parameter IDs and attachment mapping. All controls use `SliderAttachment`, `ComboBoxAttachment`, or `ButtonAttachment` to `AudioProcessorValueTreeState`.

## DSP

- **Compressor**: Measured curves from the analyzer (`output/fetish_v2/`, `output/lala_v2/`). FET mode uses threshold (dB), ratio, attack/release with envelope smoothing from `timing.csv`. Opto (LALA) uses threshold 0–100 with a gentler curve; gain reduction is scaled so LALA can sound more aggressive.
- **Curve data**: Loaded at runtime from the plugin bundle (`Contents/Resources/CurveData/`), or from `OMBIC_COMPRESSOR_DATA_PATH` / project path if not bundled. CMake POST_BUILD copies `output/fetish_v2` and `output/lala_v2` into the VST3 and (on macOS) re-copies the bundle to the user plugin folder.
- **Neon saturator**: Stochastic gain modulation plus drive-based soft saturation (tanh). Drive and Intensity control depth and how hard the signal is driven; at high Intensity the saturator can be fully overblown. Always in the signal path; Mix is dry/wet.
- **Signal path**: Saturator → compressor (fixed order), then output gain. Implemented by `emulation::MVPChain` (FET or Opto), `emulation::NeonTapeSaturation`, and final output gain in the processor.
