# OMBIC Compressor — GUI Design Analysis

Product-design analysis of the current plugin UI: the science behind it, how it lines up with research, what’s possible in JUCE, and recommended next steps.

---

## 1. Science Behind the Current GUI

### 1.1 Meter ballistics (VU-style)

**What’s implemented:** One-pole smoothing on all meter and GR readouts.

- **MeterStrip:** `smoothedInDb_ += ballisticsCoeff_ * (inDb - smoothedInDb_)` with `ballisticsCoeff_ = 0.12f` at 25 Hz.
- **CompressorSection:** Same coefficient for the GR bar and the “X.X dB” readout; `GainReductionMeterComponent` uses the same 0.12f.
- **Editor:** `startTimerHz(25)`; timer reads processor atomics and triggers repaint on the meter components only.

**Science:**  
Classic VU meters use ~300 ms rise and fall so that:

- **Rise** is fast enough to show transients.
- **Fall** is slow enough to be readable and to reflect perceived loudness rather than raw peaks.

One-pole smoothing `y += α(x - y)` has time constant τ = 1/α in samples. At 25 Hz update rate, α = 0.12 ⇒ τ ≈ 1/0.12 ≈ 8.3 updates ⇒ ~333 ms, which matches the “~300 ms” target in your research and in the OBS/FabFilter-style references. So the current ballistics are **intentionally VU-like** and appropriate for gain-reduction and level feedback.

### 1.2 Transfer curve (input vs output)

**What’s implemented:** `TransferCurveComponent` plots:

- **X-axis:** Input level (dB), -50…0 dB.
- **Y-axis:** Output level (dB), same range.
- **1:1 reference:** Dashed diagonal (no compression).
- **Curve:** Below threshold, out = in; above threshold, `out = thresh + (in - thresh) / ratio` (or gentler LALA ratio for mode 0).
- **Operating point:** Red dot at current (input level, output level) from processor atomics.

**Science:**  
This is the standard **transfer function** (input–output map) for a compressor. The flattening above threshold shows ratio at a glance; the red dot shows where the signal sits on that curve. It matches the Sonible/FabFilter “pro plugin” pattern and gives immediate feedback for threshold and ratio without needing to interpret numbers alone.

### 1.3 Thread and update model

- **Audio thread:** Processor writes `inputLevelDb`, `gainReductionDb`, `outputLevelDb` (e.g. `std::atomic<float>`).
- **Message thread:** Editor timer (25 Hz) reads those atomics and updates smoothed values and calls `repaint()` on the meter strip, transfer curve, and GR meter only—not the whole editor.

So metering and curve are **thread-safe** and **efficient**: no locks, and repaint is limited to the parts that change.

### 1.4 Visual hierarchy and grouping

- **Related controls grouped:** Compressor (mode + threshold/ratio/attack/release + GR), Saturator (drive/intensity/tone/mix), Output (makeup).
- **Cards:** Each section is a card (white fill, 3px ink border, 20px radius, 8×8 hard shadow) with a blue header strip and bold title, matching the OMBIC style guide.
- **Layout:** Fixed proportions in `resized()` (e.g. compressor ~55% width, then saturator, output column, meter strip). Resizable editor with a single layout pass.

This follows the research advice: “Group related controls,” “break rigid grids with subtle spacing,” “prominent knobs/sliders for key parameters.”

### 1.5 Look and feel

- **OmbicLookAndFeel** overrides sliders (linear and rotary), combo box, buttons, labels, and group outline.
- **Sliders:** Track (line), fill (blue), thumb (circle, ink stroke; blue when mouse-over). Vertical fill from bottom for level, horizontal for typical knobs.
- **Buttons:** Hard shadow, 3px border, blue/yellow/red by state; **mouse-over** already adds a blue highlight stroke on buttons.
- **Colors and typography:** OMBIC palette (red header, blue section headers, ink borders/shadows, teal/red for meters); Trash font when available, else system bold.

So the “depth” and “mouse-over” mentioned in the research are **partially** there: slider and button hovers exist; **section cards** do not yet have hover/depth.

---

## 2. Research vs Implementation

| Research recommendation | Status | Notes |
|-------------------------|--------|--------|
| Group related controls | Done | CompressorSection, SaturatorSection, OutputSection, MeterStrip. |
| Numeric GR + bar meter | Done | GR label + vertical bar in CompressorSection and MeterStrip. |
| Meter scale (0 / -20 / -40 dB) | Done | Scale labels on MeterStrip. |
| Input / output meters with GR | Done | MeterStrip: In, GR, Out. |
| Resizable, scalable UI | Done | Editor resizable; layout in `resized()`. |
| Custom palette (OMBIC) | Done | OmbicLookAndFeel, red/blue/teal/ink. |
| VU-style ballistics (~300 ms) | Done | One-pole α ≈ 0.12 at 25 Hz. |
| Transfer curve (in vs out) | Done | TransferCurveComponent with 1:1 reference and red dot. |
| Mouse-over / depth (highlights, shadows) | Partial | Sliders and buttons have hover; **cards do not**. |
| Graphical GR history | Optional | Not implemented; scroll/strip of GR over time. |

So the only missing “research-backed” polish is **mouse-over/depth on cards**; **GR history** is an optional extra.

---

## 3. What’s Possible with JUCE

- **Layout:** Current code uses manual `setBounds()` in `resized()`. You can keep that or move to **FlexBox** / **Grid** for more flexible or proportional resizing and clearer min/max sizes.
- **LookAndFeel:** Already used for all controls. You can add more overrides (e.g. `drawTooltip`, focus rings) or refine existing ones (e.g. rotary tick marks, tooltips with parameter value + unit).
- **Hover and depth:**  
  - **Cards:** Components can override `mouseEnter` / `mouseExit`, set a “hovered” flag, and in `paint()` use a different shadow (e.g. 6×6) and a blue outline. No need for CSS-like transforms; shadow + outline is enough for “depth” and matches the style guide.  
  - **Sliders:** Already show hover (thumb and track colour); you could add a subtle glow or outline in `drawLinearSlider`/`drawRotarySlider` when `slider.isMouseOverOrDragging()`.
- **GR history:** A small `Component` that keeps a circular buffer or deque of GR values (e.g. last N seconds at 25 Hz). In `paint()`, draw a path or strip (e.g. left = oldest, right = newest). Processor would need to expose or stream GR per tick (you already have it); the editor would push samples in the timer. JUCE’s `Path` or simple line drawing is enough.
- **Transfer curve:** You could add a **knee** (soft transition) to the curve drawing for modes that support it, or add axis tick labels; the current math (threshold + ratio) is correct for a hard knee.
- **Accessibility:** JUCE supports accessibility; you can attach descriptions and roles to sliders/labels so screen readers and host automation stay consistent.

---

## 4. Recommended Approach

**Priority 1 — Mouse-over / depth on section cards (high impact, low effort)**  
- Add hover state to the card-style sections (Compressor, Saturator, Output, MeterStrip, TransferCurve).  
- On hover: keep the same card rect, but draw shadow 6×6 instead of 8×8 and add a 2px blue outline (OMBIC “hover border highlight”).  
- Implement by: `mouseEnter`/`mouseExit` → set bool → `repaint()`. In `paint()`, branch on that bool for shadow size and stroke colour.  
- Aligns with research (“depth via highlights … mouse-over effects”) and style guide (“Hover border highlight: --ombic-blue”).

**Priority 2 — Optional: GR history strip**  
- Add a thin “GR over time” strip (e.g. below the main GR meter or in the meter strip).  
- Ring buffer of GR values in the editor, filled from `timerCallback()`; draw as a single path or filled region.  
- Improves “readable feedback on compression amount and timing” for power users; not required for the core research checklist.

**Priority 3 — Layout and scaling**  
- If you need more flexible or DPI-aware layouts later, refactor `resized()` to FlexBox/Grid and use a single scale factor from `getBounds()` or a shared constant.  
- Current fixed-pixel layout is fine for the default size; resizable already works.

---

## 5. Summary

- **Science:** Meter ballistics are intentionally ~300 ms (one-pole, α ≈ 0.12 at 25 Hz). Transfer curve correctly shows input vs output and current operating point. Thread model (atomics + message-thread timer, targeted repaint) is sound. Grouping and OMBIC card/LnF match best practices.
- **Research:** All “Apply to OMBIC” items are done except card-level **mouse-over/depth**; GR history is optional.
- **JUCE:** Supports the above; next step is to add hover/depth to section cards, then optionally GR history and layout refinements.

Implementing **card hover/depth** is the recommended first step to fully align the UI with your research and the OMBIC style guide.
