# Curve Data: What It Is, How It’s Used, and Why It Feels Subtle

This doc answers: **What is the curve data doing? Is it “max range of parameters” or “one snapshot”? Why do some parameters feel barely noticeable?**

---

## 1. What you’re confused about (the disconnect)

You have:

- **Curve data** – CSVs/JSON that describe how the reference hardware behaved at specific settings.
- **Plugin parameters** – Threshold, ratio, attack, release, etc., that the user can move.

The disconnect is:

- **Curve data is not “one snapshot”.** It’s a **set of discrete snapshots** (a few threshold × ratio × input_level points, plus timing and FR/THD).
- **Parameters don’t “override” the curves.** They **select or interpolate between** those snapshots. So:
  - Curve data = **calibration / reference behaviour** (the “family” of curves and character).
  - Parameters = **where you are inside that family** (threshold, ratio) or **how the envelope moves** (attack/release).

So: curve data gives the **shape and character** the plugin is allowed to have; parameters choose **which shape and how fast it reacts**. If the data only has a few snapshots, parameter moves can feel like small steps or extrapolation, and some of the “character” (FR, THD) isn’t in the signal path at all right now.

---

## 2. What the curve data actually is

| File | FET (fetish_v2) | Opto (lala_v2) | What it measures |
|------|------------------|----------------|-------------------|
| **compression_curve.csv** | 225 rows | 225 rows | For each of a few (threshold, ratio) settings: input level (dB) → output level / gain reduction. |
| **timing.csv** | 25 rows (5×5 attack_param × release_param) | (none) | Maps knob positions → attack_time_ms, release_time_ms for the envelope. |
| **frequency_response.csv** | 90 rows (30 freqs × 3 drive levels) | Same | Magnitude (and optionally phase) vs frequency and level. |
| **thd_vs_level.json** | 7 levels | Same | THD% and harmonics vs level. |

**FET compression curve in practice:**

- **Threshold:** 3 values: -24, -18, -12 dB.
- **Ratio:** 3 values: 4, 8, 12.
- **Timing in the CSV:** All rows use the same attack_ms/release_ms (0.5 ms, 400 ms). So the **static** transfer (input → gain reduction) was measured at **one** attack/release; there are no different “curve snapshots” per attack/release.
- So you have **9 static curves**: 3 thresholds × 3 ratios.

**Opto compression curve:**

- **Threshold:** 3 values: 25, 50, 75 (on a 0–100 scale).
- No ratio / attack / release in the curve data (opto style).

So: curve data is **several measurements**, not a full continuous mapping of every parameter. Parameters are used to **interpolate or choose** between these.

---

## 3. How the plugin uses the curve data (impact on sound)

### 3.1 Compression curve → gain reduction

- **Used for:** The **static** mapping: “for this block’s input level and current threshold/ratio, how much gain reduction?”
- **Implementation:** `MeasuredCompressor::gainReductionDb(threshold, inputDb, ratio, {}, {})` — note **attack and release are always `{}`**. So the curve lookup uses **only (threshold, ratio, input_db)**. Attack/release from the CSV are never used in the transfer curve.
- **Interpolation:**
  - **Threshold:** Blended between the two nearest curves (so you get smooth(ish) behaviour between -24, -18, -12 dB).
  - **Ratio:** Only the **nearest** of the 3 ratios (4, 8, 12) is used. The code blends only by threshold; when the two nearest keys share the same threshold, it returns the first curve and does **not** blend by ratio. So **ratio is effectively 3 discrete steps**, not a smooth sweep.
- **Impact:** The “compressor sound” (how much GR at what level, knee, hardness) comes from these 9 curves. So curve data **is** impactful for the **amount and shape** of gain reduction. But because ratio isn’t interpolated, moving the ratio knob can feel like nothing… nothing… then a step.

### 3.2 Timing curve → envelope

- **Used for:** Converting **knob values** (attack_param, release_param) into **envelope time constants** (attack_time_ms, release_time_ms) for the one‑pole envelope follower.
- **Implementation:** `getAttackReleaseMs(attackParam, releaseParam)` does **nearest-neighbour** in the timing table; no interpolation. So you get one of 25 (attack, release) combinations for FET.
- **Current data issue:** In the repo’s `timing.csv`, **release_time_ms is empty** for all rows, and **attack_time_ms** is only 0 or 10.667 ms. So:
  - Release falls back to the raw release param (treated as ms).
  - Attack is either 0 or ~10.67 ms from the table.
  So the **envelope motion** is only partly driven by real timing data; the rest is fallbacks. That can make attack/release feel “generic” or not very tied to the hardware.

### 3.3 Frequency response and THD (character)

- **Designed use:** FR = EQ colour from the unit; THD = saturation/harmonics from `thd_vs_level.json`.
- **Current plugin:** In `ensureChains()`, the chains are created with **`characterFr = false`** and **`characterThd = false`**. So **FR and THD are not in the signal path**. The plugin loads the data directories but does **not** create `FRCharacter` or `THDCharacter`. Only the compression curve + envelope + Neon (and makeup, etc.) affect the sound.
- **Impact:** A big part of “this hardware’s sound” (tone, weight, grit) is supposed to come from FR + THD. With those off, you only get the **gain-reduction shape** from the curve data, not the **tone** from the analyzer. So the curve data is underused: the part that would make it clearly “that unit” is disabled.

---

## 4. Direct answers

**Is the curve data “a couple of measurements” or “complete emulation”?**  
It’s **a set of discrete measurements** (9 transfer curves for FET, 3 for Opto, one timing grid, FR and THD tables). It’s enough to **drive** an emulation, but the current implementation uses only part of it (no FR/THD), and interpolation is limited (ratio and timing are nearest-neighbour or fallback).

**What is the curve data doing for us?**  
- **Compression curve:** Defines the **family** of static transfer curves (threshold × ratio × input level). Parameters choose **which curve** (and interpolate threshold only). So it’s giving the **range of behaviours** the plugin can have, not a single snapshot.  
- **Timing:** Intended to give the **range of envelope speeds** (attack/release) per knob; in practice the CSV is incomplete so fallbacks dominate.  
- **FR/THD:** Would give the **tone/character**; currently unused.

**Is it giving “max range of parameters” or “one snapshot”?**  
- **Snapshot:** Each row in the CSV is one snapshot (e.g. “at threshold -24 dB, ratio 4, this input → this GR”).  
- **Range:** The **set** of snapshots (e.g. 3 thresholds × 3 ratios) defines the **range** of curves the plugin can represent. Parameters then **select/interpolate within** that range (threshold smoothly, ratio in 3 steps, timing via nearest neighbour when data exists).

**Why do some parameters sound barely noticeable or too subtle?**  
- **Ratio:** Implemented as nearest-neighbour among 3 values (4, 8, 12). No smooth blend, so it can feel like 3 steps.  
- **Attack/Release:** Timing data is sparse and missing release; envelope is partly fallback, so hardware-specific “feel” is weak.  
- **Overall “sound”:** FR and THD (the main “colour” from the analyzer) are turned off, so you mainly hear gain reduction shape + Neon, not the full measured character.

---

## 5. What’s missing for the outcome you want

If the goal is “this plugin clearly sounds like the reference hardware”:

1. **Use the character data:** Enable `characterFr` and `characterThd` (or equivalent) so `frequency_response.csv` and `thd_vs_level.json` are applied. That’s where a lot of the “sound” lives.
2. **Interpolate ratio:** Blend gain reduction between the two nearest ratio curves (similar to threshold), so ratio feels continuous instead of 3 steps.
3. **Fix or expand timing data:** Populate `timing.csv` with real attack_time_ms and release_time_ms for the 5×5 grid (or add interpolation), so attack/release knobs map to real hardware-like envelope times instead of fallbacks.
4. **Optional:** More curve snapshots (e.g. more threshold/ratio points) would make parameter sweeps smoother and closer to the hardware; the current 3×3 is the minimum to get a “family” of curves. See **CURVE_GRID_SAMPLING_THEORY.md** for why 3 is undersampled and the 20×20 recommendation; see **CURVE_ANALYZER_IMPROVEMENTS.md** for what to improve in the analyzer pipeline itself (timing, FR/THD, schema, repeatability).

In short: the curve data **is** the calibration that defines the emulation’s range and character. Right now the plugin uses it **partially** (transfer curve + broken timing, no FR/THD), and the interpolation is **coarse** (ratio and timing). Fixing that would make the curve data much more impactful and the parameters more obviously tied to the “sound” you want.
