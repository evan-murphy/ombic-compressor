# Neon Bulb GUI — Design Spec (locked)

**Status:** Locked  
**Date:** 2025-02-15  
**Reference mockup:** `design-spec-files/neon-bulb-mockup.html` — **Option 5 (Tube)**

---

## 1. Summary

The Neon Saturation section uses a **tube** layout: a horizontal tube outline with the **waveform as the filament** running through it. The filament is a time-domain waveform (amplitude over time). All four parameters (Drive, Intensity, Tone, Mix) drive visible feedback in the display.

---

## 2. Layout and structure

- **Section:** Neon Saturation card (existing: pink header “NEON SATURATION”, Drive / Intensity / Tone / Mix controls).
- **Display area:** One “scope” region above the sliders (existing height e.g. 110 px). Contains:
  - **Tube outline:** A horizontal rounded rectangle (pill shape), stroke only, no fill. Centered vertically in the display. Colour: `pluginBorder`, low opacity (~0.6).
  - **Filament:** Two overlapping elements drawn *inside* the tube:
    - **Glow:** Thick stroke of the waveform path (blur or wide stroke), neon pink.
    - **Line:** Thinner stroke of the same path, neon pink, on top.

- **Waveform source:** In the plugin, use **post-saturation** audio (or sidechain when SC Listen is active) to build the path. In the mockup, synthetic waveform is used for demo; the *mapping* of parameters to visual is what we lock in.

---

## 3. Parameter → visual mapping (locked)

All four parameters must have a **pronounced** effect. Values below are 0–1 (plugin normalised); UI may show 0–100%.

| Parameter   | Effect | Implementation (visual only) |
|------------|--------|------------------------------|
| **Drive**  | **Shape** of the waveform. Higher drive = more saturation = more soft-clipping (flattened peaks). | When building the path, apply a soft-clip (e.g. tanh) to the signal; drive controls gain into the nonlinearity so the drawn wave flattens more as drive increases. |
| **Intensity** | **Glow strength** and **stroke thickness**. 0% = thin line, almost no glow; 100% = thick line + strong glow. | Line stroke width: `0.4 + 4 * intensity` (e.g. 0.4–4.4 px). Glow stroke width: `1 + 38 * intensity` (e.g. 1–39 px). Glow opacity (when mix > 0): `0.05 + 0.95 * intensity`. |
| **Tone**   | **Smooth vs wiggly** character. Low tone = smoother, more low-frequency content; high tone = more high-frequency detail (more peaks). | If deriving from real audio: tone can tilt a filter before the path (more bass vs more treble in the displayed signal). In synthetic demo: blend from low-freq to high-freq content. |
| **Mix**    | **Dry/wet visibility.** 0% = no saturated signal = filament **off** (invisible). 100% = full saturated = filament **on** (fully visible). | **Line opacity = mix** (0 → 1). **Glow opacity = mix * (0.05 + 0.95 * intensity)**. No other opacity from CSS or L&F should override; mix must fully control visibility. |

---

## 4. Colours and style

- **Tube outline:** `OmbicLookAndFeel::pluginBorder()`, stroke only, opacity ~0.6.
- **Filament (glow + line):** Neon saturation accent — `juce::Colour(0xFFe85590)` (same as current neon pink). No default opacity on the path so that **Mix** and **Intensity** control opacity explicitly.
- **Background:** Existing scope background `#080a12` (e.g. `juce::Colour(0xFF080a12)`), rounded rect, 10 px radius, 1 px border per existing §7.

---

## 5. Dimensions (from mockup)

- Display area: width = full card width minus padding; height = 110 px (existing scope height).
- Tube: horizontal pill centered in that area. Mockup: rect height 34 px, corner radius 17 px, inset from plot area (e.g. 8 px horizontal, vertical center).
- Waveform vertical range: ±14 px from center (so total “amplitude” ~28 px inside the tube). Scale incoming signal to this range.

---

## 6. Waveform path (reference for synthetic / demo)

For a **synthetic** demo or when no audio is present, the “tube waveform” can be generated from a blend of low- and high-frequency sines, with drive applying a soft-clip:

- **Low (smooth):** e.g. `sin(2πx + φ1)*0.6 + sin(4πx + φ2)*0.3`
- **High (wiggly):** e.g. `sin(8πx + φ3)*0.5 + sin(16πx + φ4)*0.35 + sin(24πx + φ5)*0.15`
- **Blend:** `y_raw = low*(1 - tone) + high*tone`
- **Drive (saturation shape):** `gain = 1 + drive*5`, then `y = tanh(y_raw * gain) / tanh(gain)`, normalised to ±1 then scaled to vertical pixels.

In the **plugin**, the path should be built from **actual post-saturation (or sidechain) samples**; drive/tone then affect the saturation and optional filter in the DSP, and the drawn path reflects the result.

---

## 7. Implementation notes (JUCE)

- **Component:** `SaturatorSection::ScopeComponent` (or equivalent) is the owner of the tube + filament drawing.
- **Sliders:** Use existing `driveSlider`, `intensitySlider`, `toneSlider`, `mixSlider` from `SaturatorSection`. Read normalised values (0–1) in `paint()` and apply the formulas in §3 and §5.
- **Opacity:** Do not rely on a default opacity from `OmbicLookAndFeel` for the filament paths; set alpha explicitly from mix and intensity so that mix 0 = invisible, mix 1 = full visibility.
- **SC Listen:** If the existing “SC Listen” mode is kept, it can override the tube with a teal sidechain trace as it does now; when SC Listen is off, draw the tube + filament with the mapping above.

---

## 8. Acceptance

- [ ] Tube outline (horizontal pill) is visible and centered in the scope area.
- [ ] Filament is a single waveform path with a glow layer and a line layer.
- [ ] **Drive** changes the *shape* of the waveform (more flattened at high drive).
- [ ] **Intensity** changes glow strength and stroke thickness (thin/dim at 0%, thick/bright at 100%).
- [ ] **Tone** changes smooth vs wiggly character (low = smooth, high = detailed).
- [ ] **Mix** controls visibility: 0% = filament off, 100% = filament fully on; no other opacity overrides.

---

**Document version:** 1.0  
**References:** `neon-bulb-mockup.html` (Option 5), `SaturatorSection.cpp` / `SaturatorSection.h`, `OmbicLookAndFeel`, `neon_bulb_saturation_research_spec.md`.
