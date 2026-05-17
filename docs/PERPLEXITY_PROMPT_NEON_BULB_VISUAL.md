# Perplexity prompt: Neon bulb visual design for audio plugin

Copy-paste the block below into Perplexity to get design and art references for representing a neon-bulb saturation process in a plugin GUI.

---

## Prompt (copy below)

```
I'm designing the GUI for an audio plugin that digitally models the saturation and harmonic character of a vintage neon bulb limiter (the kind used in broadcast/recording). The plugin has a "Neon Saturation" section with controls like Drive, Intensity, Tone, and Mix. Right now the section is just a card with sliders and a pink header.

I want to explore more interesting visual representations of this process:

1. **Art and design references**: What artists, movements, or graphic design work use neon or glowing-tube aesthetics in a stylized or abstract way (not literal neon signs)? Think poster art, album art, UI design, or product design that suggests "neon glow," "gas discharge," "warm saturation," or "electrical character" without being a literal bulb or sign. Include any design systems or iconography that abstract vacuum tubes, valves, or glow into simple shapes or motifs.

2. **Representing the process with graphic design**: How can we visually communicate "signal goes in → gets saturated/harmonically enriched by a nonlinear element → comes out" in a single graphic or small illustration? What metaphors do other audio plugins or hardware use (e.g. tube glow, meter deflection, waveform distortion)? Are there good examples of "before/after" or "input → process → output" diagrams in product or UI design that feel more like art than wiring diagrams?

3. **Abstract bulb or tube iconography**: What are elegant, minimal, or bold ways to suggest a neon bulb or tube in a logo, icon, or small on-screen graphic? I'm open to: simplified cross-sections, glow halos, filament/tube silhouettes, or purely abstract shapes that evoke "warm saturation" or "electrical glow." References from industrial design, sci-fi UI, or vintage electronics branding would help.

4. **Constraints**: The graphic will live in a flat, modern plugin UI (no 3D or skeuomorphic knobs). Prefer ideas that work as 2D vector art (lines, shapes, gradients or solid color) and that fit a small card header or a compact "process" graphic (e.g. next to the section title or above the sliders). The brand uses a limited palette: red, blue, teal, yellow, purple, pink, and black/white.

Please suggest 3–5 concrete visual directions (with artist names, movements, or example products where possible) and how each could be adapted into a simple graphic that represents "neon saturation" in this context.
```

---

## How to use the results

- Use the references (artists, movements, example UIs) as mood boards for a custom **drawable** or **image** in the Neon Saturation card (e.g. a small icon next to "Neon Saturation", or a simple process diagram).
- If Perplexity returns icon or shape ideas, you can implement them in the plugin with `juce::Path` or a small embedded SVG/PNG in the SaturatorSection header or above the sliders.
- The prompt asks for 2D-friendly, flat-style ideas that fit your existing OMBIC palette and card layout.
