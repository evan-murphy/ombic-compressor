# v1 vs v2 Analyzer Output Comparison

**Run date:** After v2 implementation. v1 = `*_standard` (old order, 0.5 s THD, 8192 FFT Hanning). v2 = `*_v2` (FR→compression→THD→timing, 3 s THD, exact-period FFT).

---

## What Changed (methodology)

| Stage | v1 | v2 |
|-------|----|----|
| **Order** | Compression → Timing → FR → THD | **FR → Compression → THD → Timing** |
| **THD tone** | 0.5 s per level | **3 s** per level (steady-state) |
| **THD FFT** | 8192 samples, Hanning window | **4800** samples (100 cycles at 1 kHz / 48 kHz), rectangular (exact-period, no leakage) |

Compression, timing, and FR use the same signals and processing; only **order** of execution changed, so those outputs are **identical** between v1 and v2 (verified: compression CSV rows match).

---

## THD: Are v2 outputs better?

**Yes.** v2 THD is more accurate because:

1. **Exact-period FFT** — Integer number of cycles in the analysis block removes spectral leakage, so the fundamental and harmonics land on single bins and power is not smeared. v1’s 8192 samples at 48 kHz = 170.67 cycles of 1 kHz → leakage and under/over-estimation.
2. **Longer tone** — 3 s allows the compressor/saturator to reach steady-state before we measure; 0.5 s may not have fully settled at some settings.

**Effect:** v2 typically reports **slightly higher THD%** at low levels and **more consistent harmonic levels** across levels, because we’re no longer diluting harmonic power with leakage.

---

## FETish: THD v1 vs v2 (same plugin, same levels)

| level_db | v1 THD% | v2 THD% | Delta (v2 − v1) |
|----------|---------|---------|------------------|
| -40 | 0.0146 | 0.0327 | +0.018 |
| -34 | 0.0146 | 0.0341 | +0.020 |
| -28 | 0.0146 | 0.0341 | +0.020 |
| -22 | 0.0147 | 0.0341 | +0.019 |
| -16 | 0.015 | 0.0343 | +0.019 |
| -10 | 0.0242 | 0.0405 | +0.016 |
| -4 | 0.1462 | 0.1539 | +0.008 |

v2 reports **~2× higher THD** at low levels and **~5% higher** at -4 dB. Harmonic bins (e.g. H2 at -4 dB: v1 -99.29 dB, v2 -92.38 dB) show v2 capturing more harmonic power in the correct bins instead of leaking into neighbors.

---

## LALA: THD v1 vs v2

| level_db | v1 THD% | v2 THD% | Delta |
|----------|---------|---------|-------|
| -40 | 0.0001 | 0.0012 | +0.0011 |
| -34 | 0.0001 | 0.0012 | +0.0011 |
| -28 | 0.0002 | 0.0012 | +0.001 |
| -22 | 0.0009 | 0.0015 | +0.0006 |
| -16 | 0.0068 | 0.007 | +0.0002 |
| -10 | 0.0532 | 0.0533 | ~0 |
| -4 | 0.4226 | 0.4227 | ~0 |

At **high level** (-10, -4 dB) v1 and v2 match; at **low level** v2 reports higher THD (less leakage, cleaner measurement). So v2 is **more accurate at low levels** without changing the high-level character.

---

## By how much better?

- **Compression / FR / Timing:** Unchanged; v2 only reorders, so no numeric improvement, but order now matches best practice (linear → static NL → dynamics).
- **THD:**  
  - **Accuracy:** v2 removes FFT leakage and uses steady-state; the *shape* of THD vs level and relative harmonics (e.g. H2 vs H3) is more reliable.  
  - **Magnitude:** At low levels v2 THD is often **~2× (FETish) or ~10× (LALA at -40 dB)** higher than v1 because v1 underestimated (leakage). At high levels the two are close.  
  - So “by how much” = **v2 THD is the better estimate**; the delta is “v1 was under-reporting by roughly a factor of 2 at low levels” for FETish, and similar for LALA at very low levels.

---

## Data we have after running both (v2)

| Directory | Plugin | Files | Rows / points |
|-----------|--------|-------|----------------|
| `output/fetish_v2/` | FETish | frequency_response.csv, compression_curve.csv, thd_vs_level.json, timing.csv | FR: 90; Compression: 225; THD: 7; Timing: 25 |
| `output/lala_v2/` | LALA | frequency_response.csv, compression_curve.csv, thd_vs_level.json | FR: 90; Compression: 225; THD: 7; Timing: skipped (opto) |

**Legacy (v1):** `output/fetish_standard/`, `output/lala_standard/` — same structure; THD in those dirs is the older (leakage-affected) version. Use **v2** outputs for emulation and any THD-based character modeling.
