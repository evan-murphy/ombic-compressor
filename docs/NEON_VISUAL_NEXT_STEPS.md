# Neon Saturation Visual — Next Steps (from Perplexity research)

Based on **Perplexity/What artists, movements, or graphic design work us.md** (your 1–4 prompt results), here are focused suggestions and one concrete implementation direction.

---

## What the research gives you

- **Movements:** Synthwave (neon horizons, grids), Vaporwave (pastel neons, glitch), Cyberpunk (high-contrast glows). **Abstract tube motifs:** geometric shapes with warm inner glow, minimal vectors, “gas discharge” feel.
- **Process metaphor:** “Signal in → glowing tube/waveshaper → harmonically rich out.” **Plugin refs:** Tube glow (Brainworx HG-2, Wave Arts Tube Saturator 2, Softube Saturation Knob), meter deflection, before/after waveform.
- **Iconography:** Silhouette (capsule + inner glow, electrode dots); cross-section (cylinder + radial glow); abstract (U or wavy line + bloom); filament (zigzag in tube + rays); halo burst (dot + elliptical halos).

---

## Suggested direction for OMBIC

**Constraint:** Flat 2D UI, OMBIC palette (no literal neon signs, no 3D). You want something that reads as “neon saturation” without clutter.

**Best fit:** A **single small graphic** in the Neon Saturation card that suggests “signal through a glowing element” using one of these:

1. **Capsule silhouette (recommended first)**  
   - Elongated oval/capsule (like a simplified tube or bulb).  
   - Soft inner fill (e.g. pink/teal at low alpha) + ink outline.  
   - Optional: two small “electrode” circles at the ends.  
   - Fits in the section header (next to “Neon Saturation”) or as a small motif above the sliders.  
   - Easy to draw with `juce::Path` (addRoundedRectangle or addEllipse); no assets.

2. **Abstract “U” or wavy line with bloom**  
   - Single curved path (U or sine) with a slightly thicker stroke and a second stroke at lower opacity for “glow.”  
   - Suggests “energized path” / signal flow.  
   - Also Path-based; could sit above the sliders as a horizontal strip.

3. **Horizontal flow (in → tube → out)**  
   - Three elements: short line (in) → capsule or diamond (process) → short line (out).  
   - Closer to the “input → process → output” idea from the research.  
   - Slightly more complex; good as a second pass.

---

## Concrete implementation idea (capsule motif)

**Placement:** In **SaturatorSection**, right of the “Neon Saturation” title in the pink header bar (or left of the text). Size about 20–24 px tall so it doesn’t dominate.

**Shape:**  
- Capsule: rounded rectangle or two semicircles + rect (e.g. 8–10 px wide, 20 px tall, corner radius 4).  
- Fill: `ombicPink` at 0.3–0.5 alpha (or teal for contrast).  
- Stroke: `ink` 1–1.5 px.  
- Optional: two small circles (2 px) at top and bottom inside the capsule as “electrode” dots.

**Code:** Implement in `SaturatorSection::paint()`, after drawing the header rect and before or after `drawText("Neon Saturation", ...)`. Build a `juce::Path` for the capsule (and dots if you want), then `g.fillPath()` and `g.strokePath()` with your colours. Position the path so it sits in the header (e.g. x = 14, y = 6, w = 10, h = 20) with the title text starting after it (e.g. 32–36 px from left).

**Later:** If you add a “drive” or “amount” visual, the capsule fill alpha or a second “glow” path could scale with the drive parameter (more drive → stronger glow), similar to “tube lights up with drive” from the research.

---

## What to do next

1. **Implement the capsule motif** in `SaturatorSection` as above (no new dependencies; Path + paint only).  
2. **Optional:** Add a second, even subtler “glow” path (same shape, larger, lower opacity) behind the capsule for a soft halo.  
3. **If you want a stronger “process” metaphor:** Add the horizontal flow (in → capsule → out) as a small strip above the Drive/Intensity/Tone/Mix sliders in a later iteration.  
4. **Keep the research doc** for future art direction (e.g. when you want to refine the icon or add a second graphic elsewhere).

If you want, the next step can be a small patch that adds the capsule (and optional glow) to `SaturatorSection::paint()` with exact coordinates and colours matching your current header layout.
