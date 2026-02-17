# OMBIC Compressor v2 — Product GUI Spec (Outline)

**Status:** Outline for fill-in. Authoritative design brief for v2; v1 preserved as fallback.  
**Role:** Single source of truth for layout, sizing, hierarchy, and host behavior.  
**Tone:** One idea per section. We do X; we don’t do Y.

---

## 1. Purpose and scope

- **Product:** OMBIC Compressor — VST3/AU (and AAX if relevant) plugin: compressor (Opto/FET) + sidechain filter + neon saturation + output/metering.
- **v2 goal:** Redesign GUI from scratch for clarity, one visual system, and maximum host compatibility. v1 code and UI remain available as fallback; v2 is a new editor and layout.
- **Out of scope for this spec:** DSP changes, new compressor modes, new formats. This spec is **GUI and interaction only**.
- **The v2 GUI exists to:** Show level, compression, and saturation in one place — and put every control where it belongs.

---

## 2. Design principles (locked)

- **One visual system.** Dark plugin UI only. No mixing web (light) and plugin (dark) in the same doc; plugin palette is defined here and in §7.
- **Host-first.** Sizing, scaling, and layout are designed so the plugin behaves predictably in every host and DPI. We do not assume 100% scale or a fixed aspect ratio.
- **Single source of truth.** All dimensions and layout rules live in this spec (and one place in code). No “spec says 110px, GAPS says 82px”—we state one value or one rule.
- **Controls first.** The elements you twist (knobs, pills, faders) are the visual heroes; headers and labels support, they don’t dominate.
- **Vector and data-driven.** We draw with `Path`, type, and gradients. We drive every control from APVTS and every meter/curve from processor state (atomics + timer). No bitmap-only UI, no duplicate state.
- **One central display.** Level, GR, transfer curve, and saturation (the tube/filament) live in the main view. We do not duplicate “where the signal is” across two areas.

---

## 3. Window and host compatibility

- **Default size:** 960 × 540 px. Larger than v1; not smaller.
- **Resize policy:** **Resizable.** `setResizable(true, true)` with `setResizeLimits(960, 540, 1600, 900)`. Layout in `resized()` only, using FlexBox or Grid from `getLocalBounds()`. No structural layout in timer.
- **Code contract:** One place defines base size (e.g. `constexpr int kBaseWidth`, `kBaseHeight`). All layout derives from `getLocalBounds()` or from these constants. No magic numbers in three files.
- **Scaling:** We use JUCE’s scaling. We do **not** apply a global `Graphics` transform to “scale the whole UI.” We do **not** force process-wide DPI awareness on Windows.
- **Drawing:** Vector (`juce::Path`, gradients, `drawText`), embedded fonts. No bitmaps for primary controls unless we ship multiple resolutions.
- **OpenGL:** We use it only if we need GPU for heavy meters/curves, and only after verifying HiDPI and multiple hosts. Default: software renderer.
- **Child windows:** Pop-ups, menus, colour pickers are parented to the editor’s top-level component. No OS-level hacks.
- **Testing checklist:** VST3 (+ AU, AAX if relevant). Hosts: [ ] Reaper, [ ] Live, [ ] Cubase, [ ] Studio One, [ ] FL Studio, [ ] Logic, [ ] PluginHost. Windows: 100%, 125%, 150%, 200% scaling. Actions: open/close GUI, move between monitors, host zoom if present.
- **Test hosts (check off when done):** Reaper, Live, Cubase, Studio One, FL Studio, Logic, PluginHost. Windows: 100%, 125%, 150%, 200% scaling.

---

## 4. Layout and hierarchy

- **High-level structure (order of vertical strips):**
  1. Header bar (plugin name, optional status).
  2. Mode row (Opto / FET pills only).
  3. **Main view (the tube)** — the one central display. The main view **is** the tube: its whole background/glow is driven by saturation; inside it we draw level (In/Out), GR, transfer curve, and the waveform (filament). Height per §6. This is where the eye goes. Turn up Drive/Intensity and the whole section lights up.
  4. **Control row:** one horizontal strip of columns: Sidechain Filter | Compressor | **Neon (knobs only)** | Output.
  5. Footer (brand, version, optional).
- **Main view:** Is the tube. No separate “Neon scope” card. Saturation drives the main view’s background/glow and the filament; the Neon column is just Drive, Intensity, Tone, Mix knobs + label.
- **Content row:** Grid gap 12 px. Padding 12 px L/R, 16 px bottom. Column widths: proportional (e.g. SC Filter 0.55fr, Compressor 0.85fr Opto / 1.5fr FET min 320px, Neon 1.2fr knobs-only, Output 0.7fr). When resizable, FlexBox or Grid from `getLocalBounds()`; min widths respected.
- **Layout mechanism:** FlexBox or Grid in `resized()`. We do **not** compute section bounds in a timer.
- **Mode-dependent layout:** Opto ↔ FET can change Compressor column width; transition optional (e.g. 300 ms). Structural bounds always from `resized()`.

---

## 5. Component list and ownership

- **PluginEditor** owns: APVTS attachments, timer for meters/curve/tube, resize constrainer. Does **not** assign section bounds in timer.
- **HeaderBar:** Height 38 px. Left: “OMBIC COMPRESSOR”; right: optional status. Font/size per §7.
- **Mode row:** Height 32 px. Pills: Opto, FET (84×26 px, gap 12 px). No ComboBox in the strip.
- **Main view (one component, the tube):** The main view **is** the tube. It contains: level meters (In/Out), GR, transfer curve, and the waveform (filament). Its background/glow is driven by saturation (Drive, Intensity, Mix). Behaviour per §8.
- **Sidechain Filter section:** Card; header + body. Controls: frequency, SC Listen. Min width ~130 px.
- **Compressor section:** Card; header; body: transfer curve (or ref to main view), Opto single knob or FET four knobs, GR readout/meter. Min width 320 px in FET.
- **Neon section:** **Knobs only.** Card with header “NEON” (or “NEON SATURATION”); body: Drive, Intensity, Tone, Mix. **No scope in this card.** The saturation visual is in the main view.
- **Output section:** Card; meters + output knob + GR readout. Meter dimensions per §6.
- **Footer:** Height 26 px; padding 8 px vert, 20 px horiz. “Ombic Sound” | version | “Stereo”.

---

## 6. Sizing and spacing (one table)

- All values at **default window size** unless marked “when resizable: [rule]”.

| Element | Value | Notes |
|--------|--------|------|
| Window default | 960 × 540 | §3; min 960×540, max 1600×900 |
| Header height | 38 px | |
| Mode row height | 32 px | |
| Pills: width × height | 84 × 26 px, gap 12 px | |
| Grid gap (columns) | 12 px | |
| Content padding L/R, bottom | 12 px, 16 px | |
| Footer height | 26 px; padding 8 vert, 20 horiz | |
| **Main view height** | **120 px** (default) | When resizable: proportional, min 100 px. Main view = the tube; holds meters, curve, filament; background/glow from saturation §8. |
| Filament (waveform in main view) | amplitude ±14 px | Waveform path inside the tube; layout within main view TBD. §8 |
| Module card: border radius | 16 px | |
| Module card: header height | 36 px; body padding 14 px | |
| Transfer curve (in main view) | ~90 px of main view | Shared with tube; layout within main view TBD |
| Knobs: Opto (single) | 80 px diameter | |
| Knobs: FET (each) | 60 px diameter; gap 16 px | |
| Knobs: Neon (each) | 48 px diameter; gap 14 px | |
| Knobs: Output | 56 px diameter | |
| Output meters | 6 px wide, 80 px height | |
| GR readout font/size | 16 px, color-coded | |

---

## 7. Visual system (colors, type, shadow)

- **Palette (plugin dark UI):** Per OMBIC_COMPRESSOR_JUCE_SPEC §2. Background `#0f1220` (pluginBg). Surface (cards) `#181c2e` (pluginSurface). Raised (knob centers) `#222840` (pluginRaised). Text primary `#e6e9ef` (pluginText). Muted `#6b7280` (pluginMuted). Border `0x33e6e9ef` (pluginBorder). Border strong `0x59e6e9ef`. Shadow `0x80000000`. Accents: Red `#ff001f`, Blue `#076dc3`, Teal `#338fab`, Yellow `#ffce00`, Purple `#5300aa`, Pink `#e85590`.
- **Typography:** Trash (Bold 700 min; Black 900 for headers). Plugin title 18 px, 900. Module headers 13 px, 900. Param labels 9 px, 700, uppercase. Param values 14 px, 900; Opto large 20 px. Pills 11 px, 900. Footer 9 px, 700. GR value 16 px, color-coded. Letter-spacing: 1.5 px on plugin title if feasible; else not required.
- **Shadows:** Hard offset only: 8 px horizontal, 8 px vertical, 0 blur, colour pluginShadow. No soft blur.
- **Knob style:** Sweep 240° (−2.356 to 2.356 rad). Track `0x14ffffff`. Value arc = module accent (Compressor blue, Neon pink, Output purple). Center fill pluginRaised. Pointer 2.5–3 px. Stroke 5 px (large), 4 px (small).

---

## 8. Neon saturation — main view is the tube (locked)

- **Chosen concept:** **The main VU is the tube.** The main view is not a box with a small tube inside it; the **main view itself** is the tube. Its whole area is the bulb. Level meters, GR, transfer curve, and the waveform (filament) are drawn **inside** that one container. Saturation (Drive, Intensity, Tone, Mix) drives:
  - **The container:** Background glow / ambient color of the whole main view (e.g. inner glow, tint) so the whole section is visibly “lit” by saturation.
  - **The filament:** Waveform shape (Drive), glow and thickness (Intensity), smooth vs wiggly (Tone), visibility (Mix).
- So: one shape, one story. Turn up Drive or Intensity and the **entire** main view responds — not just a small scope. The Neon **section** remains knobs only (Drive, Intensity, Tone, Mix + header).
- **Parameter → visual mapping:**  
  - **Drive:** Waveform shape (soft-clip) **and** strength of main-view background glow/tint.  
  - **Intensity:** Filament glow and stroke thickness **and** how much the main view “lights up” (e.g. inner glow, saturation tint).  
  - **Tone:** Smooth vs wiggly (low = more low‑freq, high = more detail).  
  - **Mix:** Filament visibility (0 = off, 1 = full) **and** overall saturation presence in the main view (0 = neutral/dark, 100% = full glow).  
- **Main view as tube:** Background/base `#080a12`; saturation adds inner glow or tint (e.g. neon pink `#e85590` at alpha driven by Drive/Intensity/Mix). Tube outline optional (e.g. subtle `pluginBorder` pill or none so the glow is the edge). Filament colour `#e85590`; opacity from Mix and Intensity.
- **Waveform source:** Post-saturation audio; synthetic when no signal. SC Listen can override to sidechain if applicable.
- **We do not:** Draw a separate small tube inside the main view. The main view **is** the tube. We do not show a logo inside it. One tube = one main view.

---

## 9. Motion and feedback

- **Meter/curve/tube update:** Processor writes input level, GR, output level to atomics. Editor timer at 25 Hz (or 60 Hz for snappier feel) reads and repaints only: main view (meters, curve, tube), Output meters, Compressor GR. We do **not** repaint the whole editor every tick.
- **VU ballistics:** One-pole smoothing ~300 ms rise/fall (e.g. coeff 0.12 at 25 Hz).
- **Transfer curve:** Redraw on parameter change; live dot from processor atomics each timer tick.
- **Layout animation:** Optional: column proportion change on Opto ↔ FET over 300 ms. Structural bounds always from `resized()`.

---

## 10. Data and state

- **All controls:** Attached via APVTS. Parameter IDs and ranges in processor `createParameterLayout()`. No duplicate state in the editor.
- **Meters, curve, tube:** Read-only from processor atomics. Variable block size acceptable.
- **Hit areas:** Generous; independent of pixel-perfect artwork for host scaling.

---

## 11. Testing and acceptance

- **Host compatibility:** Pass §3 checklist (hosts + Windows scaling). See TESTING_PROTOCOL.md.
- **Visual acceptance:** Sizes in §6 match implementation. No clipped text or controls at 960×540. At min size 960×540 and max 1600×900, no overlap.
- **Regression:** v1 remains buildable and loadable; v2 is separate editor/target.

---

## 12. References and supersession

- **This spec supersedes for v2:** Layout and sizing in OMBIC_COMPRESSOR_JUCE_SPEC; neon placement in NEON_BULB_GUI_SPEC and UX_IMPROVEMENTS_SPEC §2. Tube lives in main view; Neon section = knobs only.
- **Reference only (no authority):** GUI_SPEC.md, GUI_BEST_PRACTICES_RESEARCH, PERPLEXITY_GRAPHICS_RECOMMENDATIONS, NEON_VISUAL_NEXT_STEPS.
- **Implementation guide:** OMBIC_COMPRESSOR_JUCE_GUIDE for API and drawing; must align with §3–§6 and §8.
- **v1 fallback:** v1 editor and assets remain available; v2 is the new default when implemented.

---

## Document control

- **Version:** 1.2 (implementation complete)  
- **Date:** 2025-02-15  
- **Build:** v2 editor is implemented. CMake option `OMBIC_USE_V2_EDITOR` (default ON) selects v2; set to OFF to build v1 editor.
- **Implementation status (100%):**
  - §3: 960×540, setResizeLimits(960,540,1600,900), layout in resized() only, juce::Grid for content row, no global scale, vector/fonts.
  - §4: Header → pills → main view (tube) → control row (Grid) → footer. Main view height proportional (min 100px, leaves ≥80px for control row).
  - §5: All components; Neon section knobs only (setScopeVisible(false)); header 38px, pills 84×26 gap 12, footer 26px padding 8/20.
  - §6: All sizing table values: main view 120px (resizable proportional), filament ±14px, card radius 16px header 36px body 14px, Opto 80 FET 60 gap 16, Neon 48 gap 14, Output 56, meters 6×80, GR 16px.
  - §7: Palette, typography (title 18, module headers 13, param labels 9, GR 16), shadow 8×8, knob sweep 240°.
  - §8: Main view is the tube; glow + filament driven by Drive/Intensity/Tone/Mix; Neon section = knobs only.
  - §9: Timer 25 Hz; repaint main view, Output, Compressor GR, Saturator; VU ballistics ~300ms.
  - §10: All controls via APVTS; meters/curve from atomics.
  - §11: Visual acceptance per §6; v1 buildable with OMBIC_USE_V2_EDITOR=OFF.
