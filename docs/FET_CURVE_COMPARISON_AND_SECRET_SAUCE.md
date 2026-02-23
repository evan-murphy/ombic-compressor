# FET Curve Data: How It Differs, What’s In the Dataset, and the “Secret Sauce”

This doc compares **all FET curve data** in the repos: the **shipped** curve (fetish_v2 from Analog Obsession FETish) vs **reference** derived curves (UAD 1176 Rev A, AE, LN Legacy, LN Rev E, Purple MC77). It infers what the outward-facing FET mode *is*, and suggests **reasonable tweaks** for different sound.

---

## 1. What Exists Where

| Source | Location | Param space | Use |
|--------|----------|-------------|-----|
| **FETish (shipped)** | `output/fetish_v2/compression_curve.csv` | `threshold` (dB), `ratio`, `input_db` → `output_db`, `gain_reduction_db` | **Bundled**; plugin uses this for FET mode. |
| **UAD 1176 Rev A** | `derived_curves/vst3_uad_1176_rev_a/merged_transfer_curves.csv` | T, C, G (0.25–1.0), `input_db` → `output_db`, `gain_reduction_db` | Reference only. |
| **UAD 1176 AE** | `derived_curves/vst3_uad_1176ae/merged_transfer_curves.csv` | Same T/C/G | Reference only. |
| **UAD 1176 LN Legacy** | `derived_curves/vst3_uad_1176ln_legacy/` | Same | Reference only. |
| **UAD 1176 LN Rev E** | `derived_curves/vst3_uad_1176ln_rev_e/merged_transfer_curves.csv` | Same | Reference only. |
| **Purple MC77** | `derived_curves/au_purple_audio_mc_77_plugin_alliance/merged_transfer_curves.csv` | Same T/C/G | Reference only. |

- **fetish_v2**: From **vst-analyzer** run on **Analog Obsession FETish** (VST3). Grid: **20 thresholds** (-30 to 0 dB), **20 ratios** (4–20), **25 input levels** (-60 to 0 dB in 2.5 dB steps). Fixed **0.5 ms attack**, **400 ms release**. ~10k rows.
- **Derived**: From **analyze-captured-batch** on **plugin_captures** WAVs. **T/C/G** come from filenames (e.g. `_T0.25_C0.50_G0.50.wav`). Same test signal (stepped levels + transients); **16 T/C combos** (T,C ∈ {0.25, 0.50, 0.75, 1.00}, G=0.50), **30 blocks** per combo. So **T and C are normalized knobs**, not “threshold dB” or “ratio” — they map differently per plugin.

---

## 2. How the Curves Differ (Numerically)

### 2.1 FETish (shipped) — one slice

At **threshold -30 dB**, **ratio 4:1** (first curve in fetish_v2):

| input_db | output_db | gain_reduction_db |
|----------|-----------|-------------------|
| -20      | -35.74    | **15.74**         |
| -10      | -33.24    | **23.24**         |
| 0        | -30.69    | **30.69**         |

So: **aggressive** — once above threshold, GR grows quickly and substantially (e.g. 0 dB in → 30.69 dB GR).

### 2.2 UAD 1176 Rev A (derived) — same nominal “medium” ballpark

At **T0.25, C0.50, G0.50** (block 0–9, stepped levels):

| input_db | output_db | gain_reduction_db |
|----------|-----------|-------------------|
| -23.07   | -28.49    | **5.42**          |
| -18.4    | -27.31    | **8.9**           |
| -13.74   | -26.42    | **12.68**         |
| -9.07    | -25.64    | **16.57**         |

So at **similar** input levels, 1176 Rev A at “T0.25 C0.50” gives **less** GR than FETish at -30 dB / 4:1 (e.g. ~8.9 dB at -18.4 in vs FETish ~15.74 at -20 in). **FETish is more aggressive** than this 1176 “medium” setting.

At **T0.25, C0.25** (more “ratio” or “all in” on 1176):

| input_db | output_db | gain_reduction_db |
|----------|-----------|-------------------|
| -23.07   | -56.25    | **33.18**         |
| -18.4    | -55.07    | **36.66**         |
| -9.07    | -53.4     | **44.33**         |

So 1176 “heavy” (T0.25 C0.25) is **crush** — in the same ballpark as or heavier than FETish at -30/4 at high levels.

### 2.3 UAD 1176 LN Rev E

At T0.25 C0.50: -23.07 → GR 6.02, -18.4 → 8.89. **Slightly less** GR than Rev A at same T/C; same character.

### 2.4 UAD 1176 AE

At T0.25 C0.25: -23 → 20.31, -9 → 30.53 (less GR than Rev A at same T/C). At T0.25 C0.50: -23 → 10.61, -18.4 → 13.58 (more GR than Rev A). So **revision matters**: AE is a different balance (less at “all in”, more at “medium”).

### 2.5 UAD 1176 T0.50 C0.50 (“mid” threshold, “mid” ratio)

From `T0.50_C0.50_G0.50/captured_transfer_curve.csv`: -23.07 → 2.63 dB GR, -18.4 → 6.52 dB. **Softer** than FETish at -30/4 (which gives ~15.74 at -20).

### 2.6 Purple MC77

At **T0.25 C0.25** and **T0.25 C0.50**: `output_db ≈ input_db`, **gain_reduction_db ≈ 0** (or tiny). So at the **same nominal T/C/G**, MC77 in this capture is effectively **bypass**. T/C map to different behaviour per plugin (or that capture had a very high threshold on MC77).

---

## 3. Structural Differences (Why Comparison Is Tricky)

- **Param space**: FETish = **physical** (threshold dB, ratio, one attack/release). Derived = **normalized** T/C/G from REAPER; **no single mapping** from T/C to threshold/ratio across plugins.
- **Grid**: FETish = 20×20 threshold×ratio, 25 input levels. Derived = 16 T/C combos × 30 blocks; input levels come from the test signal (not a uniform -60..0 grid).
- **Knee / aggression**: For “medium” settings, **FETish gives more GR** than UAD 1176 at T0.25 C0.50 or T0.50 C0.50. So the **shipped** FET character is on the **aggressive** side of the 1176 family in the dataset.
- **Revision / unit variation**: Rev A vs LN Rev E vs AE vs MC77 show that “FET” is a **family**: same topology, different amounts of GR and knee for similar nominal settings.

---

## 4. What Can Be Inferred: FETish vs Captured Data

- **FETish (fetish_v2)** is **one specific 1176-style curve set**: Analog Obsession’s FETish plugin, measured at a dense threshold×ratio grid.
- Compared to the **captured** FET family:
  - **More aggressive** than “medium” 1176 (T0.25 or T0.50 C0.50): more GR for similar input level.
  - **In the same ballpark** as 1176 “all in” (T0.25 C0.25) at high levels.
  - **Not comparable** to MC77 at same T/C — MC77 at those captures is effectively no compression.
- So the **outward-facing FET mode** is “FETish’s transfer curve”, not an average of UAD/MC77. The “secret” is: **one strong, 1176-style, fairly aggressive character** with smooth interpolation over threshold and ratio.

---

## 5. What Else Is in the Dataset (But Not in the Sound)

- **fetish_v2** also has:
  - **timing.csv**: attack/release grid (currently timing has issues / fallbacks in the plugin).
  - **frequency_response.csv**: magnitude vs frequency and drive level.
  - **thd_vs_level.json**: THD% and harmonics vs level.
- In the plugin, **only the compression curve** is used for the FET transfer; **FR and THD are not in the signal path** (`characterFr` / `characterThd` off). So the **tone** (saturation, colour) from the analyzer is **not** part of the current “secret sauce” — only the **gain-reduction shape** is.

---

## 6. Reasonable Tweaks for the Outward-Facing FET Mode

If you want **different sound** while staying data-informed:

1. **Softer / “vintage gentle”**
   - Slightly **reduce GR** above threshold (e.g. scale GR by 0.85–0.9) or blend with a softer curve (e.g. 1176 T0.50 C0.50–style) so it sits between FETish and “medium” 1176.
   - Or **widen the knee** (more gradual transition) so it feels less grabby at the threshold.

2. **More “grabby” / Rev A–style**
   - Slightly **increase GR** in the knee region (e.g. 5–10% more GR just above threshold) to lean toward 1176 “all in” without changing high-level ratio behaviour.

3. **Use the rest of the dataset**
   - **Enable FR and THD** from fetish_v2 so the plugin applies `frequency_response.csv` and `thd_vs_level.json`. That would add the **tone** (EQ colour, harmonics) of the captured FETish and differentiate more from a clean digital compressor.

4. **Second FET flavour**
   - Ship an **alternative** FET curve (e.g. derived from 1176 LN Rev E or a blend) as a **sub-mode or preset** (“FETish-style” vs “1176 LN–style”) so users can choose character without changing code.

5. **Fix timing**
   - Populate/use **timing.csv** properly so attack/release feel like the measured FETish (or chosen reference) instead of fallbacks.

---

## 7. One-Line Summary

**Secret sauce (as discerned from this dataset):** The outward-facing FET mode is **FETish’s static transfer curve** — a single, aggressive 1176-style character with dense threshold×ratio interpolation. It is **more aggressive** than “medium” UAD 1176 in the reference data and in the same ballpark as 1176 “all in” at high levels. The rest of the dataset (FR, THD, timing) is currently **unused** in the signal path, so the only thing that defines the sound is that **gain-reduction shape**.

---

## 8. FET Presets / Modes: User-Familiar and Audibly Different

Based on the curve comparison, these **named FET characters** map to changes users would recognize. The numbers below show they are **truly audibly different** (multi‑dB differences in GR at typical working levels).

### 8.1 Suggested presets (names and character)

| Preset name | User association | Data backing | Typical GR vs current (FETish) | Audible? |
|-------------|------------------|---------------|----------------------------------|----------|
| **FETish** (default) | “Modern” / “Punch” / current behaviour | fetish_v2 as shipped | Baseline (~15.7 dB at -20 dB in, -30 thresh, 4:1) | — |
| **Rev A** | “All buttons in” / grabby / classic 1176 | UAD 1176 Rev A T0.25 C0.25 | **More** GR in knee and at high level (e.g. 33–44 dB at -23 to -9 in) | **Yes** — ~2× GR in transition, similar at full crush |
| **LN / Vintage** | “Softer” / “1176 LN” / gentle FET | UAD 1176 T0.50 C0.50 | **Less** GR (e.g. ~2.6–6.5 dB at -23 to -18 in vs FETish ~15.7 at -20) | **Yes** — roughly half (or less) the GR at medium levels |
| **AE** (optional) | “1176 AE” — different balance | UAD 1176 AE | Less at “all in”, more at “medium” than Rev A | **Yes** — middle ground between Rev A and LN |

- **Rev A**: Feels **grabby** and **fast** — more gain reduction through the knee and into heavy compression. Same ballpark as FETish at very high input; **more** GR in the -25 to -10 dB range. Very familiar to anyone who has used an 1176 “all in.”
- **LN / Vintage**: **Clearly softer** — similar threshold/ratio settings produce **noticeably less** compression (order of 5–10+ dB less GR at typical levels). Familiar as “vintage” or “gentle 1176.”
- **AE**: Audibly different from both: less “slam” at extreme settings, a bit more action at medium settings. Good optional fourth flavour.

So: **Rev A** and **LN/Vintage** are both strongly supported by the data and are **audibly distinct** from each other and from current FETish. AE is a reasonable optional fourth.

### 8.2 Implementation options

**Option A — Single curve + runtime scaling (recommended first step)**  
Keep one curve set (fetish_v2). Add a **FET character** (or “curve flavour”) parameter, e.g.:

- **0 = LN / Vintage**: scale GR by ~0.45–0.55 (so at -20 dB in you get ~7–8 dB GR instead of ~15.7). Optionally soften knee (e.g. reduce GR when “just above” threshold).
- **0.5 = FETish** (default): scale 1.0.
- **1 = Rev A**: scale GR by ~1.1–1.2 in the knee region (e.g. boost GR when input is 3–12 dB above threshold); keep high-level GR similar or slightly higher.

No new files; one new parameter; easy to tune by ear. Gets you “softer / default / grabby” with minimal code and no pipeline changes.

**Option B — Precomputed alternate curves**  
Generate two extra CSVs (e.g. from scaled fetish_v2 or from derived 1176 data mapped to threshold/ratio grid):

- `fetish_rev_a/compression_curve.csv` (or equivalent)
- `fetish_ln/compression_curve.csv` (or equivalent)

Plugin loads one of three curve sets based on preset. More authentic to “real” Rev A / LN curves but requires a small pipeline or script to produce the CSVs in the same format as fetish_v2.

**Option C — Blend two curves**  
If you later add a second full curve set (e.g. “softer” in fetish_v2 format), blend: `gr = (1 - flavour) * gr_soft + flavour * gr_fetish`. Same threshold/ratio/interpolation as now; one blend parameter.

### 8.3 Normalizing output level for easy auditioning (Auto Gain)

**Goal:** When switching FET presets (LN ↔ FETish ↔ Rev A) with **Auto Gain** on, **output volume should not change** — only the character (amount and shape of GR). That way users can A/B the presets without loudness confounding.

**Why it works:** Each preset produces a **different** amount of gain reduction. If Auto Gain adds **makeup = actual GR** (the GR that was just applied), then LN (less GR) → less makeup, FETish → baseline makeup, Rev A (more GR) → more makeup. Output level stays ~same as input in all cases. So normalization is **not** about changing the curves; it's about **driving Auto Gain from actual GR** instead of a preset-independent estimate.

**Current behaviour:** Auto Gain uses `estimateMakeupDb(mode, thresholdRaw, ratio, …)` — a **parameter-based** formula. It does **not** depend on the FET preset or the real curve. So today, switching presets would change actual GR but **not** the makeup estimate → **output level would jump** (LN louder, Rev A quieter).

**Change to normalize:** When Auto Gain is on and the mode uses a measured curve (FET, Opto, VCA), use **actual gain reduction** for makeup instead of `estimateMakeupDb`. After the chain has run, use `gainReductionDb` (from the chain's `getLastGainReductionDb()`): e.g. `makeupTotal += actualGrDb`. Keep `estimateMakeupDb` for PWM and fallbacks. Then: **Bypass** → GR = 0 → makeup 0. **FETish / LN / Rev A** → each gets makeup = its actual GR → **output level normalized** for easy auditioning. Optional: smooth GR for makeup to avoid pumping on transients.

### 8.4 Recommendation

1. **Ship three FET presets:** **FETish** (default), **Rev A**, **LN / Vintage** — names users already know.
2. **Implement first** with **Option A** (GR scaling + optional knee tweak) so the difference is clearly audible without new assets.
3. **Normalize for auditioning:** When Auto Gain is on, use **actual GR** for makeup in FET (and Opto/VCA) so switching presets does not change output level.
4. **Tune by ear** against the doc numbers: e.g. “LN” should sit around half the GR of FETish at -20 dB in; “Rev A” should feel grabby through the knee.
5. **Optionally** add **AE** as a fourth preset using a middle scaling (or a second curve later).
6. **Later**, if you want maximum authenticity, add Option B (precomputed Rev A / LN curves) and optionally switch presets to load different curve sets instead of scaling.
