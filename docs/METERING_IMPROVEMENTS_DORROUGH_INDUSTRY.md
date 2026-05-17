# Metering Improvements: Dorrough / Industry-Aligned Monitoring

Your current metering is "not useful" for the same reasons many plugin meters fall short: **block RMS only**, **single ballistic** for everything, **no peak visibility**, and **no stereo**. This doc spells out what’s wrong and what to change to get Dorrough-like, industry-aligned monitoring.

---

## 1. Why the current metering feels useless

| Issue | What you have | Why it hurts |
|-------|----------------|--------------|
| **Level type** | Block RMS only (one value per processBlock) | Transients and peaks are averaged away. You never see overloads or punch. |
| **Ballistics** | Single one-pole (same attack/release) ~300 ms | Good for VU “average” only. No fast response to peaks, no slow decay to read the value. |
| **Peak vs average** | One number: RMS | Pro monitoring shows **both** peak (fast) and average (VU/RMS). You only show average. |
| **Refresh rate** | 25 Hz timer | Slightly sluggish; 40–60 Hz feels snappier. |
| **Peak hold** | None | You can’t see “how high did it go?” |
| **Stereo** | Mono sum only | No L/R or balance view; useless for stereo imaging. |
| **GR ballistics** | Same slow coeff as level | Gain reduction needs **faster attack** so you see when the compressor hits. |

So: you’re showing a slow, block-averaged mono level and a slow GR. That’s fine for “rough loudness” but not for **monitoring** (catching peaks, judging GR timing, or stereo).

---

## 2. What “Dorrough-like” / industry monitoring means

- **Dual ballistics:**  
  - **Peak:** fast attack (1–10 ms), slow release (300–1500 ms).  
  - **Average (VU-like):** slower, ~300 ms rise and fall (IEC 60268-17 style).
- **Peak + average (or RMS) shown together** so you see both “instant” level and “loudness.”
- **Peak hold:** hold the maximum for 1–3 s so you can read it.
- **Level measurement:** peak (or PPM-style) **and** average/RMS per channel; not only block RMS.
- **Stereo:** at least L/R (or L+R / L−R for width).
- **Gain reduction:** fast attack so the GR meter tracks the compressor’s action; optional short hold.

---

## 3. Improvements (in order of impact)

### A. Processor: expose peak and average (or RMS), and stereo (high impact)

**Current:** One `inputLevelDb` and one `outputLevelDb` per block, block RMS, mono sum.

**Change:**

- Compute **per-block peak** (max abs sample across channels and samples) and **per-block RMS** (or keep current RMS as “average”).
- Write **peak** and **average** (RMS) in dB to atomics, e.g.:
  - `inputPeakDb`, `inputRmsDb` (or `inputAverageDb`)
  - `outputPeakDb`, `outputRmsDb`
- Optionally **stereo:** `inputPeakDbL`, `inputPeakDbR` (and same for output). If you keep one “main” meter, use max(L,R) or sum for backward compatibility.

**Where:** `PluginProcessor::processBlock` (level computation and `.store()`), `PluginProcessor.h` (new atomics). All existing UI that only needs “one number” can keep using RMS or peak (e.g. MeterStrip can switch to peak for the bar).

**Why:** This single change makes the meter **informative**: you see peaks and average instead of one smoothed average.

---

### B. Dual ballistics: peak (fast attack, slow release) vs average (VU-like) (high impact)

**Current:** One coefficient (e.g. 0.12 at 25 Hz) for all meters; same rise and fall.

**Change:**

- **Peak path:**  
  - Attack: reach 99% in ~5–10 ms (e.g. coefficient derived from 5 ms at your timer rate).  
  - Release: 300–600 ms decay (e.g. coefficient for 400 ms).
- **Average path:**  
  - Keep ~300 ms attack and release (current behaviour is fine for “average”).
- Implement as **two state variables per meter** (peak and average), or use a proper ballistic (e.g. separate attack/release coefficients; peak = max(instant, peak_smoothed) with different decay).

**Where:** `MeterStrip`, `MainVuComponent`, `OutputSection::LevelMeterComponent`, `CompressorSection::GainReductionMeterComponent`: replace single `smoothedXxxDb_` with peak + average state and two coefficients (attack fast, release slow for peak).

**Why:** Matches PPM/VU behaviour and Dorrough-style “see peaks and loudness.”

---

### C. Gain reduction: faster attack, optional hold (high impact for “usefulness”)

**Current:** GR uses same 0.12 smoothing as level; no hold.

**Change:**

- **Faster attack:** e.g. 10–20 ms so the GR bar moves with the compressor.
- **Slower release** than attack (e.g. 200–400 ms) so it’s readable.
- **Optional peak hold:** hold max GR for 1–2 s; decay or reset on timer.

**Where:** All GR meters: `MeterStrip`, `CompressorSection::GainReductionMeterComponent`, `OutputSection` GR readout, `MainVuComponent` GR. Use separate coeffs (or a small GR-specific ballistic) and a `grHoldDb_` / `grHoldTime_` for hold.

**Why:** GR becomes “what the compressor is doing right now” instead of a sluggish average.

---

### D. Peak hold for level meters (medium impact)

**Current:** No hold; needle/bar only shows current smoothed value.

**Change:**

- For **peak** display only: hold the current peak value for 1–3 s, then let it decay (or reset on a button).
- Optional: small “peak hold” indicator (e.g. a marker or different colour at hold level).

**Where:** Same components that display peak (after A and B). One `peakHoldDb_` and `peakHoldTimer_` per meter.

**Why:** Lets you read “how high did it go?” without staring at the meter.

---

### E. Stereo: L/R or L+R and L−R (medium impact)

**Current:** Input and output are mono sum.

**Change:**

- Processor: compute and store **per-channel** peak (and optionally RMS) for at least L and R, e.g. `inputPeakDbL`, `inputPeakDbR`, same for output.
- UI: either **two bars** (L | R) or **one bar with two segments** (e.g. left half L, right half R). Optional: switch to “sum / difference” (mid/side) like some Dorrough modes.

**Where:** `PluginProcessor` (per-channel level computation), `MeterStrip` / `OutputSection` / `MainVuComponent` (draw two bars or one dual bar).

**Why:** Essential for balance and width; expected in “industry” monitoring.

---

### F. Refresh rate and smoothing (low impact)

**Current:** 25 Hz timer; one-pole smoothing in timer.

**Change:**

- Increase timer to **40–50 Hz** for meters (and GR).
- Keep ballistics in the timer (no need to move to audio thread); just use the new dual ballistic (B) and GR (C) coefficients at the new rate.

**Where:** `startTimerHz(25)` → `startTimerHz(45)` (or 50) in `MeterStrip`, `MainVuComponent`, and any other meter component.

**Why:** Slightly snappier and more “pro” feel; small CPU cost.

---

### G. Scale and range (low impact)

**Current:** In/Out 0 to −60 dB; GR 0 to −20 or −40 dB.

**Change:**

- Consider **−3 dB** or **0 dB** at top and **−60** or **−70** at bottom; mark **−18** or **−20** (typical reference) with a tick or colour change.
- GR: keep 0 to −20 or −40; ensure scale is linear in dB so “6 dB GR” is easy to read.

**Where:** `MeterStrip::paint` (dbToY, labels), `OutputSection` / `CompressorSection` GR scale. No change to processor.

**Why:** Aligns with “reference level” workflows.

---

## 4. Implementation order

1. **A** – Peak + average (and optionally stereo) from processor.  
2. **B** – Dual ballistics (peak vs average) in UI.  
3. **C** – GR: faster attack, slower release, optional hold.  
4. **D** – Peak hold for level.  
5. **E** – Stereo L/R (or L+R / L−R) once A exposes per-channel data.  
6. **F** – 40–50 Hz refresh.  
7. **G** – Scale/range tweaks.

After A and B, the meter is already much more “industry aligned.” C makes GR actually useful for compression monitoring.

---

## 5. Code touch points (summary)

| Area | File(s) | Change |
|------|---------|--------|
| Level source | `PluginProcessor.cpp` (processBlock), `PluginProcessor.h` | Peak + RMS (and optional L/R) computation; new atomics. |
| Meter strip | `MeterStrip.cpp/.h` | Dual ballistic (peak + average); optional peak hold; use peak or both. |
| Main VU | `MainVuComponent.cpp` | Same dual ballistic; GR faster attack/release. |
| Output section | `OutputSection.cpp` (LevelMeterComponent), GR readout | Dual ballistic; GR hold; optional L/R bars. |
| Compressor GR | `CompressorSection.cpp` (GainReductionMeterComponent) | Faster GR attack, slower release; optional hold. |
| Timer | All meter components | `startTimerHz(45)` (or 50). |

---

## 6. References

- **VU:** IEC 60268-17 — 300 ms attack and decay.  
- **PPM:** Fast attack (~1.7 ms time constant), slow decay (~650 ms to −20 dB).  
- **Dorrough:** Dual display (peak + average), fast peak ballistics, optional stereo modes (L/R, sum/diff).  
- **Practical read:** [Sound Au – VU and PPM Metering](https://www.sound-au.com/project55.htm).

Once peak + average are available from the processor and the UI uses dual ballistics and faster GR, your metering will be much more aligned with Dorrough and industry-style monitoring and actually useful for compression and gain reduction.
