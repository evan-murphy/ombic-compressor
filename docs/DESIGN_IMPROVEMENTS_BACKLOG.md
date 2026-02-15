# Design improvements backlog

Ideas from GUI_SPEC, GUI_DESIGN_ANALYSIS, PERPLEXITY_GRAPHICS_RECOMMENDATIONS, and GUI_BEST_PRACTICES_RESEARCH that could still improve the plugin UI. Not in strict priority order.

---

## Done recently

- Knobs instead of vertical sliders (all sections)
- Less twitchy knobs: velocity-based mode off, skew on key parameters (threshold, ratio, drive, tone, mix, intensity, output)
- Neon bulb motif + glow in Saturator header
- Label padding (6px left), section header indent (16px)
- Mode names: Opto / FET
- Logo in header, pink/purple accents

---

## High value, reasonable effort

- **Scrolling GR history** – Ring buffer of GR (and optionally input) over last ~200–500 ms, drawn as a path or strip (e.g. in MeterStrip or under transfer curve). Perplexity research and GUI_DESIGN_ANALYSIS both recommend it.
- **60 Hz UI timer** – Bump editor/meter/curve refresh from 25 Hz to 60 Hz for snappier feel; keep VU ballistics (~300 ms) by leaving smoothing coeff as-is or re-tuning.
- **Peak GR label** – Short hold/decay “Peak: -X dB” next to the GR readout (e.g. in CompressorSection).

---

## Polish

- **Tooltips** – Parameter name + unit on hover (JUCE tooltip or LookAndFeel).
- **Document data flow** – One-line comment or GUI_SPEC note: “Meters/curve: atomics from processor, timer poll, no processBlock locks.”
- **Transfer curve** – Optional soft-knee curve (when DSP supports it); optional interactive drag on curve to adjust threshold/ratio (v2).

---

## Optional / later

- **In → capsule → out** strip above Saturator sliders (small process diagram).
- **Layout** – FlexBox/Grid for more flexible or DPI-aware resizing if needed.
- **Accessibility** – Roles/descriptions for screen readers and host automation.

---

## Reference

- Science and research map: `GUI_DESIGN_ANALYSIS.md`
- Perplexity “what to borrow”: `PERPLEXITY_GRAPHICS_RECOMMENDATIONS.md`
- Component hierarchy: `GUI_SPEC.md`
