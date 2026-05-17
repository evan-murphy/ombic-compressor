# Neon and VCA — FAQ

Answers to common questions about VCA compression, Neon controls, and signal flow.

---

## Pre-build checklist (compressor modes + VCA curve)

Before building and testing, confirm:

1. **Compressor mode → UI controls** (all correct):
   - **Opto (0):** Threshold + Compress/Limit toggle only.
   - **FET (1):** Threshold, Ratio, Attack, Release.
   - **PWM (2):** Threshold, Ratio, Speed.
   - **VCA (3):** Threshold, Ratio.

2. **Mode index:** Processor and editors use the same conversion: normalized choice value `v` → index `(int)(v * 3 + 0.5)` so 0, 1, 2, 3 map to Opto, FET, PWM, VCA.

3. **VCA curve data:**
   - **Source:** `output/dbcomp_vca/compression_curve.csv` must exist (and does in this repo).
   - **Build:** CMake POST_BUILD copies `output/dbcomp_vca` into the VST3 bundle at `Contents/Resources/CurveData/dbcomp_vca` when that file exists.
   - **Runtime:** Plugin finds curve data from (1) bundle `CurveData`, or (2) dev fallback: project root / CWD / `OMBIC_COMPRESSOR_DATA_PATH` with `output/dbcomp_vca`.
   - If VCA curve is missing at runtime, VCA mode does no compression (only optional Neon).

---

## 1. “I don’t hear any VCA compression”

Two main causes:

**A) Threshold is too high**  
VCA maps the 0–100% threshold to reference units -1..3. At **100%** you’re at the top of that range, so most program material stays below threshold and GR stays at 0 dB. **Lower the threshold** (e.g. 40–70%) so the input crosses threshold and you’ll hear gain reduction.

**B) VCA curve data not found**  
VCA (like FET/Opto) uses measured curve data. If the plugin can’t find `dbcomp_vca` (e.g. in the built VST3 bundle or via `OMBIC_COMPRESSOR_DATA_PATH` / project `output/dbcomp_vca`), `vcaChain_` is never created. In that case, when VCA is selected the code does **no compression** (only optional standalone Neon if enabled). So:

- **Dev:** Run from the project directory so `output/dbcomp_vca/compression_curve.csv` is found.
- **Build:** Ensure the installer/build copies `CurveData/dbcomp_vca` into the plugin bundle so “Curve OK” and VCA both work in the shipped plugin.

---

## 2. What do the new Neon features do?

- **BURST (Burstiness 0–10)**  
  Adds random “burst” events to the gain modulation: short dips/spikes at a rate derived from the value (e.g. “events per second”). At **0** it does nothing. Raise it (e.g. 2–6) with **Drive** and/or **Intensity** up to hear a more obvious “neon flicker” / discharge-style modulation.

- **G MIN (85–100%)**  
  Floor on the modulation gain (minimum gain in the neon stage). **100%** = gain never goes below unity (less pump). **Lower** (e.g. 85–95%) allows the modulation to pull the level down more, so the effect is more audible, especially with strong Drive/BURST.

- **Soft Sat After**  
  When **on**, soft (tanh) saturation is applied **after** the gain modulation multiply. When **off**, saturation is applied **before** the multiply (modulate the already-saturated signal). So it changes where in the chain the saturation sits; the difference is subtle unless Drive/Intensity are up.

To hear Neon clearly: turn **Neon On**, set **Drive** and **Intensity** above 0, then adjust **BURST**, **G MIN**, and **Soft Sat After**.

---

## 3. Is Neon before or after the compressor in the circuit?

**In the actual signal path, Neon is before the compressor.**

The code uses `setNeonBeforeCompressor(true)` and the comment states “saturator always before compressor”. So the order is:

1. **Neon** (saturator)  
2. **Compressor** (Opto/FET/PWM/VCA)  
3. Iron, makeup, output

The UI layout (SC Filter → Compressor → Neon → Output) shows Neon to the right of the compressor, but that’s **visual layout only**. If you want the GUI to reflect the circuit, you could add a small “Neon → Compressor” signal-flow note or reorder the sections so Neon appears before the compressor block.

---

## Reference (code)

- VCA chain creation: `PluginProcessor::ensureChains()` — requires `dbcomp_vca/compression_curve.csv`.
- VCA threshold mapping: `processBlock()` maps threshold 0..100 → -1..3 for the measured curve.
- Neon order: `MVPChain::process()` runs `neon_->process(buffer)` then `compressor_->process(buffer)` when `neonBeforeCompressor_` is true.
- Neon params: `NeonTapeSaturation::setBurstiness`, `setGMin`, `setSaturationAfter`; used in `MVPChain::setNeonParams` and `processBlock()`.
