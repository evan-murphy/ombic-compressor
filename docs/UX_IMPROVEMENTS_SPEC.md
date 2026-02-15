# Ombic Compressor — UX Improvements Spec

**Purpose:** Actionable spec for a new agent/session. Execute the items below to reduce bloat, improve feedback, and fix the neon capsule.

**Context:** The plugin has clear hierarchy and section identity, but users report: (1) neon capsule logo still tiny, (2) no obvious visual changes when increasing drive/level/params, (3) UI feels bloated (low density, large minimum window).

---

## 1. Tighten layout (reduce bloat)

**Goal:** Higher information density; smaller default/minimum size without clipping.

| Task | Details |
|------|--------|
| **1.1** Reduce padding | In `PluginEditor::resized()` and section `resized()`/paint: reduce `pad`, `shadowPad`, and internal section padding (e.g. 12→8 where safe). Keep shadows visible. |
| **1.2** Slightly smaller headers | Section card headers are 28 px; consider 24 px and proportional font so cards feel less tall. |
| **1.3** Lower minimum window size | In `PluginEditor` constructor, reduce `minW`/`minH` (e.g. from 920×520 to ~800×460 or 820×480). Re-run layout math so Compressor + Saturator + Output + MeterStrip + curve all fit without clipping. |
| **1.4** Optional: reduce transfer curve height | Transfer curve currently gets a fixed 100 px strip; consider 80–90 px if layout is tight. |

**Acceptance:** Same controls visible; window can be smaller; layout feels denser, not cramped.

---

## 2. Neon capsule: commit or simplify

**Goal:** Either make the logo-in-capsule readable and meaningful, or remove it and keep a simple motif.

**Option A — Commit (logo as hero)**  
- Capsule large enough that logo is recognizable (e.g. capsule 28–32 px tall, logo area ~24×28).  
- One clear drive-linked effect: e.g. glow strength or fill brightness obviously increases with Drive so “tube lights up” is unmistakable.  
- Position “Neon Saturation” text so it doesn’t crowd the capsule.

**Option B — Simplify (no logo in capsule)**  
- Remove logo drawing from `SaturatorSection::paint()`; remove `ensureLogoLoaded()`, `logoImage_`, and `OmbicAssets.h` include from that file.  
- Keep capsule + pink glow (optionally still drive-linked) as the only neon motif.  
- Capsule can stay small (e.g. 12×20) as a simple icon.

**Acceptance:** Either (A) logo is clearly visible and drive-linked change is obvious, or (B) capsule is a clean simple shape with no tiny logo.

---

## 3. Make “things increasing” visible (feedback)

**Goal:** When the user turns Drive, Threshold, or level up, at least one obvious visual changes.

| Task | Details |
|------|--------|
| **3.1** Transfer curve | Curve already redraws with threshold/ratio. Ensure the **red dot** (current in/out point) moves visibly with input level. If it’s subtle, consider slightly larger dot or a short trail. Optional: one-sentence tooltip or hint that “blue curve = your settings, dot = current level.” |
| **3.2** Neon drive feedback | If keeping capsule: make drive-linked glow or fill **obviously** stronger with Drive (e.g. alpha or radius step change so 0 vs 0.5 vs 1.0 is clearly different). |
| **3.3** Meters | Confirm GR and Out meters react quickly and visibly to level and threshold (ballistics, repaint rate). No code change if already responsive; otherwise reduce smoothing or increase repaint rate (e.g. 25→40 Hz) for snappier feedback. |

**Acceptance:** User can see a clear change when turning up Drive, threshold, or input level (curve dot, neon glow, or meters).

---

## 4. Optional: compact mode

**Goal:** Allow a smaller “quick” layout for users who want less footprint.

- Add a **compact** layout or **smaller default size** (e.g. 720×420) with: single row of key controls (Threshold, Ratio, Drive, Output), smaller transfer curve, meters still visible.  
- Implement either as: (a) a second `resized()` branch when `getWidth() < 900` (or similar), or (b) a toggle “Compact” that switches layout.  
- Defer to a later sprint if time is short; 1–3 are higher priority.

---

## 5. Files to touch (reference)

- **Layout / size:** `Plugin/Source/PluginEditor.cpp` (constructor `minW`/`minH`, `resized()`), section `resized()` and padding in `CompressorSection`, `SaturatorSection`, `OutputSection`, `MeterStrip`, `TransferCurveComponent`.
- **Neon capsule:** `Plugin/Source/Components/SaturatorSection.cpp` (paint: capsule size, logo draw, glow/fill alpha vs drive), `SaturatorSection.h` (logo image, `ensureLogoLoaded` if Option A kept).
- **Feedback:** Same SaturatorSection (drive→glow), `TransferCurveComponent` (dot size/trail if any), `PluginEditor::timerCallback` (repaint rate for meters/curve).

---

## 6. Order of execution

1. **§1** — Tighten layout and lower min size (quick win, less bloat).  
2. **§2** — Choose A or B for neon capsule and implement (removes “still tiny” complaint).  
3. **§3** — Improve feedback (curve dot, neon glow, meters) so “no changes as things increase” is addressed.  
4. **§4** — Optional compact mode when 1–3 are done.

---

*Spec derived from UX review of Ombic Compressor GUI; for handoff to a new agent session.*
