# FET Presets: How It Works in Practice (Review Before Implement)

This doc is a **pre-implementation review**: what the user sees and hears, what each preset does under the hood, and a clear backend vs frontend checklist. Use it to sign off before implementing both sides.

---

## 1. How It Works in Practice (User Perspective)

### 1.1 What the user sees

- **When:** User is in **FET** compressor mode (not Opto/PWM/VCA).
- **New control:** A way to choose **FET character** — dropdown labels: **Off** | **Rev A** | **LN** (UI never shows "Fetish"; Off = default character).
- **Default:** **Off** (current behaviour; no change for existing sessions).
- **Auto Gain:** If Auto Gain is **on**, switching between Off / Rev A / LN does **not** change output volume — only the **character** of the compression (so they can A/B without loudness jumps).

### 1.2 What the user hears when they switch (same threshold/ratio/input)

| Switch to   | Subjective change |
|-------------|--------------------|
| **FETish**  | Baseline: punchy, clear GR; “modern” 1176-style (current behaviour). |
| **Rev A**   | **More** compression through the knee and at medium levels — grabby, “all buttons in,” more slam. GR meter reads higher for the same input. |
| **LN**      | **Less** compression — gentler, “vintage” 1176, more headroom. GR meter reads lower for the same input. |

- With **Auto Gain on:** Output level stays roughly the same when switching; only the **shape and amount** of compression change (transient handling, density, “grab”).
- With **Auto Gain off:** Switching presets will change output level (Rev A quieter, LN louder) because makeup is fixed or parameter-based.

### 1.3 Typical use

1. User selects FET mode, sets threshold/ratio/attack/release as usual.
2. User turns **Auto Gain** on (optional but recommended for auditioning).
3. User cycles **Off → Rev A → LN** and listens: same settings, same input, different “grab” and density.
4. User picks the character they want and continues mixing.

---

## 2. Differences in FET Mode (Backend / Signal Path)

All presets use the **same** underlying curve data (fetish_v2). The only difference is **how much** gain reduction is applied (and optionally **where** — knee vs high level). No extra CSV files in Option A.

### 2.1 FETish (default)

- **Behaviour:** Current behaviour. Look up GR from the curve; apply it. No scaling.
- **Rough GR (reference):** At -20 dB input, -30 dB threshold, 4:1 ratio → **~15.7 dB** GR.
- **Code:** `grOut = curveLookup(threshold, inputDb, ratio)` (unchanged).

### 2.2 LN / Vintage

- **Behaviour:** **Less** gain reduction — softer, gentler curve.
- **Implementation (Option A):** After curve lookup, scale GR down: e.g. `grOut = grCurve * 0.50` (tune in 0.45–0.55 range).
- **Rough result:** Same conditions as above → **~7–8 dB** GR instead of ~15.7. Clearly less compression.
- **Optional:** Slightly soften the knee (e.g. scale GR down more when “just above” threshold) for a rounder feel.

### 2.3 Rev A

- **Behaviour:** **More** gain reduction in the knee and at medium levels — grabby, “all in.”
- **Implementation (Option A):** After curve lookup, **boost** GR in the knee region. For example:
  - Define “knee region” as when input is a few dB above threshold (e.g. 3–12 dB over).
  - In that region: `grOut = grCurve * 1.15` (tune in 1.1–1.2).
  - Above that: keep `grOut = grCurve` or slightly higher so high-level slam is similar or a bit more.
- **Rough result:** At -20 dB in, -30 thresh, 4:1 → **~18 dB** GR (more grab). At 0 dB in, similar to FETish or slightly more.

### 2.4 AE (optional fourth)

- **Behaviour:** Middle ground — less slam than Rev A at extremes, more action than LN at medium levels.
- **Implementation:** e.g. `grOut = grCurve * 0.85` in knee, `* 1.0` at high level; or a single scale ~0.9. Tune by ear.

### 2.5 Summary table (backend)

| Preset  | GR vs curve          | Typical scaling / tweak      |
|---------|----------------------|-------------------------------|
| FETish  | 1:1 (unchanged)      | None                          |
| LN      | Less                 | `gr *= 0.45–0.55`             |
| Rev A   | More in knee         | `gr *= 1.1–1.2` in knee       |
| AE      | Slightly less overall| `gr *= ~0.9` or knee-only tweak |

---

## 3. Auto Gain Normalization (Backend)

- **Rule:** When **Auto Gain** is on and mode is **FET** (and ideally Opto/VCA), **makeup = actual GR** applied in that block, not `estimateMakeupDb(...)`.
- **How:** After the compressor chain runs, read `gainReductionDb` (or chain’s `getLastGainReductionDb()`). Then: `makeupTotal += actualGrDb` (with optional smoothing).
- **Result:** LN (less GR) gets less makeup; Rev A (more GR) gets more makeup → output level stays ~constant when switching presets with Auto Gain on.
- **Fallback:** Keep `estimateMakeupDb` for PWM and when actual GR isn’t available (e.g. bypass).

---

## 4. Backend vs Frontend Checklist

Use this to review before implementing.

### 4.1 Backend

- [ ] **FET character parameter**  
  - New parameter (e.g. choice: 0 = FETish, 1 = Rev A, 2 = LN; or 0–1 continuous with three named points).  
  - Stored in APVTS; saved/recalled with session.

- [ ] **GR scaling in FET path**  
  - In the place where FET GR is applied (e.g. `MeasuredCompressor::gainReductionDb` or the code that uses its result), apply a scale (and optional knee tweak) based on FET character:
    - FETish: no change.
    - LN: multiply GR by ~0.5 (tune 0.45–0.55).
    - Rev A: multiply GR by ~1.15 in knee region (tune 1.1–1.2); same or slight boost above.
    - AE (if present): e.g. ~0.9 or knee-only.

- [ ] **Auto Gain from actual GR**  
  - When Auto Gain is on and mode is FET (and Opto/VCA), add **actual** `gainReductionDb` to makeup instead of `estimateMakeupDb`.  
  - Optional: smooth the GR value used for makeup to avoid pumping.  
  - Keep `estimateMakeupDb` for PWM and fallbacks.

- [ ] **No new curve files** (Option A): All presets use the same fetish_v2 curve; only runtime scaling.

### 4.2 Frontend

- [ ] **Control for FET character**  
  - Only visible when **mode = FET**.  
  - Options: dropdown (“FETish” / “Rev A” / “LN” [+ “AE”]), or 3–4 buttons, or a single knob with three (or four) labelled positions.

- [ ] **Labels**  
  - Use names users know: **FETish** (default), **Rev A**, **LN** (or “LN / Vintage”). Optional: **AE**.

- [ ] **Default**  
  - New sessions and “reset” → FET character = FETish.

- [ ] **Parameter ID**  
  - One new param (e.g. `fet_character` or `fet_preset`) for APVTS and automation.

### 4.3 What does *not* change

- Threshold, ratio, attack, release, and all other FET controls behave the same; only the **amount/shape** of GR changes with the preset.
- Opto, PWM, VCA modes are unchanged; FET presets apply only in FET mode.
- Bypass: no compression, no GR, Auto Gain adds 0 when on.

---

## 5. Quick Reference: In Practice

| Question | Answer |
|----------|--------|
| What changes when the user picks a different FET preset? | Only how much (and optionally where) gain reduction is applied — same threshold/ratio/curve, different GR scaling. |
| Does output level change when switching presets? | With **Auto Gain on**: no (makeup = actual GR). With Auto Gain off: yes (Rev A quieter, LN louder). |
| Do we need new curve files? | No for Option A; one curve (fetish_v2), scaling in code. |
| Where is the scaling applied? | In the FET gain-reduction path, after curve lookup, before the gain is applied to the buffer. |
| What does the user need to do to compare presets? | Select FET mode, turn Auto Gain on, switch FETish / Rev A / LN and listen. |

---

## 6. References

- **Curve data and preset design:** `FET_CURVE_COMPARISON_AND_SECRET_SAUCE.md` (§8).
- **Auto Gain normalization:** same doc, §8.3.
- **Current Auto Gain / makeup:** `PluginProcessor.cpp` (`estimateMakeupDb`, makeup application), `MVPChain` / `MeasuredCompressor` (actual GR).
