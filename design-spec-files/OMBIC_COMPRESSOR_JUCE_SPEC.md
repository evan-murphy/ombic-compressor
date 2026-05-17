# Ombic Compressor — JUCE UI Specification

> **For use with:** Cursor / AI code editors building the JUCE plugin editor.
> **Reference mockup:** `ombic-compressor.html` (open in browser to see interactive behavior).
> **Brand style guide:** `OMBIC_SOUND_STYLE_GUIDE.md` (web/marketing context — this spec adapts it for a dark plugin UI).

---

## 1. Plugin Window

- **Default size:** 900 × 480 px (resizable is optional, but this is the target)
- **Background:** `#0f1220`
- **Border radius:** 20px (if windowing system supports it; otherwise square)
- **Font:** Trash (embedded .ttf files: Trash-Regular.ttf, Trash-Bold.ttf). Minimum weight is Bold (700). Use Black (900) for headers.

---

## 2. Color Palette

```cpp
// Brand accents — use exactly these values
const Colour ombicRed    { 0xFFff001f };
const Colour ombicBlue   { 0xFF076dc3 };
const Colour ombicTeal   { 0xFF338fab };
const Colour ombicYellow { 0xFFffce00 };
const Colour ombicPurple { 0xFF5300aa };
const Colour ombicPink   { 0xFFffb3c9 };

// Dark plugin surfaces
const Colour pluginBg      { 0xFF0f1220 };  // Main background
const Colour pluginSurface { 0xFF181c2e };  // Module card backgrounds
const Colour pluginRaised  { 0xFF222840 };  // Knob centers, elevated elements
const Colour pluginText    { 0xFFe6e9ef };  // Primary text
const Colour pluginMuted   { 0xFF6b7280 };  // Labels, secondary text
const Colour pluginBorder  { 0x33e6e9ef };  // ~20% white, card borders
const Colour pluginBorderStrong { 0x59e6e9ef }; // ~35% white, outer border
const Colour pluginShadow  { 0x80000000 };  // 50% black, hard offset shadow
```

### Color Assignments

| Element | Color | Notes |
|---|---|---|
| Plugin background | `pluginBg` | |
| Module card fill | `pluginSurface` | |
| Module card border | `pluginBorder` | 2px |
| Plugin outer border | `pluginBorderStrong` | 3px |
| Header bar — Compressor | `ombicBlue` | White text |
| Header bar — Neon Saturation | `#cc3a6e` → `#e85590` gradient | White text |
| Header bar — Output | `ombicPurple` | White text |
| Knob arc (compressor) | `ombicBlue` | |
| Knob arc (saturation) | `#e85590` | Pink |
| Knob arc (output) | `ombicPurple` | |
| Knob track (inactive arc) | `0x14ffffff` | ~8% white |
| Knob center fill | `pluginRaised` | |
| Parameter labels | `pluginMuted` | 9px, uppercase |
| Parameter values | `pluginText` | 14px, bold |
| GR readout | Color-coded: `ombicTeal` < 3dB, `ombicYellow` 3–6dB, `ombicRed` > 6dB | |
| Transfer curve line | `ombicBlue` | 2px stroke |
| Scope input waveform | `pluginMuted` at 60% | Clean sine |
| Scope saturated waveform | Gradient: `pluginMuted` → `#e85590` → `ombicRed` | |
| Mode pill (active) | `ombicBlue` fill, white text | |
| Mode pill (inactive) | Transparent, `pluginBorderStrong` border, `pluginMuted` text | |

---

## 3. Layout — Grid Structure

The plugin body uses a 3-column layout. Column proportions change based on compressor mode:

### Opto Mode
```
| Compressor (0.85fr) | Neon Saturation (1.6fr) | Output (0.7fr) |
```

### FET Mode
```
| Compressor (1.5fr, min 320px) | Neon Saturation (1.3fr) | Output (0.6fr) |
```

- **Grid gap:** 12px
- **Grid padding:** 12px left/right, 16px bottom
- **Transition:** Animate column widths over 300ms ease when switching modes

### Vertical Structure
```
┌─────────────────────────────────────────────────┐
│ Header bar (38px height)                         │  ← ombicRed bg
├─────────────────────────────────────────────────┤
│ Mode switcher row (padding 12px top)             │  ← Opto / FET pills
├────────────┬──────────────────┬──────────────────┤
│ Compressor │ Neon Saturation  │ Output           │
│  module    │  module          │  module          │
│            │                  │                  │
├────────────┴──────────────────┴──────────────────┤
│ Footer bar                                       │  ← "Ombic Sound" / version
└─────────────────────────────────────────────────┘
```

---

## 4. Module Cards

- **Background:** `pluginSurface`
- **Border:** 2px solid `pluginBorder`
- **Border radius:** 16px
- **Header bar:** Full-width, 8px vertical padding, 14px horizontal padding
- **Header text:** 13px, weight 900, uppercase, letter-spacing 1px, white
- **Body padding:** 14px

---

## 5. Knob Component

Each knob is an SVG-rendered rotary control with these characteristics:

### Geometry
- **Sweep range:** 240° (from 150° to 390°, where 0° = top)
- **Track:** Full 240° arc in `0x14ffffff` (8% white), round endcaps
- **Value arc:** Partial arc from start to current value position, in the module's accent color, round endcaps
- **Glow:** Same arc path, +6px wider stroke, 15% opacity, 4px blur — gives the arc a soft bloom
- **Center circle:** Filled `pluginRaised`, 1px `0x0fffffff` border
- **Pointer:** Small dot (r=2.5–3px) at the current angle, filled with accent color

### Sizes
| Context | Diameter | Stroke width | Pointer dot radius |
|---|---|---|---|
| Opto threshold (single large) | 80px | 5px | 3px |
| FET parameters | 60px | 4px | 2.5px |
| Saturation parameters | 48px | 4px | 2.5px |
| Output knob | 56px | 4px | 2.5px |

### Interaction
- **Drag:** Vertical mouse drag (up = increase, down = decrease)
- **Sensitivity:** `(max - min) / 150px` of vertical movement
- **Cursor:** Grab on hover, Grabbing while dragging

### Label + Value Layout (per knob)
```
  LABEL       ← 9px, weight 700, uppercase, pluginMuted, letter-spacing 0.8px
   [knob]     ← SVG knob
   Value      ← 14px (20px for large Opto knob), weight 900, pluginText
```
Centered column, 4px gap between elements.

---

## 6. Compressor Module

### Mode Selector
- Pills at top of plugin (outside module card), in the mode-switcher row
- Active: `ombicBlue` fill, 2px `ombicBlue` border, white text
- Inactive: Transparent fill, 2px `pluginBorderStrong` border, `pluginMuted` text
- Font: 11px, weight 900, uppercase, letter-spacing 0.5px
- Border radius: 999px (pill)
- Padding: 5px 14px
- Hover: border becomes `ombicBlue`, text becomes `pluginText`

### Transfer Curve Display
- **Size:** Full card width × 90px height
- **Background:** `pluginBg`
- **Border:** 1px solid `pluginBorder`, 10px border radius
- **Margin bottom:** 14px (before knobs)
- **Grid lines:** 6×6 grid, `0x0affffff` (4% white), 1px
- **Unity line:** Diagonal corner-to-corner, dashed (3px on, 3px off), `0x14ffffff` (8% white)
- **Curve line:** `ombicBlue`, 2px solid
- **Curve glow:** Same path, 8px stroke, `rgba(7, 109, 195, 0.2)`
- **Axis labels:** "OUT" top-left, "IN" bottom-right, 8px, weight 700, 25% white

### Curve Math
```
for each input x (0 to 1 normalized):
  if x <= threshold:
    output = x  (unity)
  else:
    output = threshold + (x - threshold) / ratio
```
Where threshold is normalized (param / 100) and ratio is the ratio param (or fixed 4.0 for Opto).

### Opto Mode Controls
- Single Threshold knob, 80px, centered
- Value text: 20px

### FET Mode Controls
- Four knobs in a row: Threshold, Ratio, Attack, Release
- All 60px diameter
- 16px gap between knob groups
- Minimum card width: 320px

### Parameters

| Param | Min | Max | Step | Default | Unit |
|---|---|---|---|---|---|
| Threshold | 0 | 100 | 0.1 | 50.0 | — |
| Ratio | 1 | 20 | 0.1 | 4.0 | :1 |
| Attack | 0.1 | 1000 | 1 | 410 | ms |
| Release | 5 | 2000 | 1 | 200 | ms |

---

## 7. Neon Saturation Module — CENTERPIECE

### Waveform Scope Display
- **Size:** Full card width × 110px height
- **Background:** `#080a12` (near-black)
- **Border:** 1px solid `pluginBorder`, 10px radius
- **Margin bottom:** 14px
- **Center line:** Horizontal at 50% height, `0x0affffff`, 1px

### Waveform Rendering
The scope shows a continuous sine wave that transitions from clean input to saturated output:

1. **Input region (0% to ~35% of width):** Clean sine wave, stroked in `pluginMuted` at 60% opacity, 1.5px line
2. **Transition + saturated region (35% to 100%):** Waveform is processed through soft clipping (tanh waveshaper), with clipping intensity increasing left-to-right proportional to `(drive * 2 + intensity) * progress`
3. **Color gradient on saturated region:** Linear gradient from `pluginMuted` (60%) → `#e85590` → `ombicRed`, with opacity increasing with drive
4. **Glow layer:** Same saturated waveform path, 10px stroke, gradient from transparent → `ombicPink` at `drive * 0.25` opacity → `ombicRed` at `drive * 0.2` opacity
5. **Box glow:** Inner box-shadow on the scope container: `inset 0 0 30px rgba(232,85,144, drive*0.12 + intensity*0.06)` plus `inset 0 0 60px rgba(255,0,31, half_that)`

### Waveform Animation
- Continuously animate (requestAnimationFrame / timer)
- Phase advances at ~0.03 radians per frame
- 6 cycles visible across the width (`Math.PI * 6`)
- Saturation is applied via: `y = tanh(y * clipGain)` where `clipGain = 1 + satAmount * progress * 3`

### Saturation Knobs
- Four knobs in a row: Drive, Intensity, Tone, Mix
- All 48px, `#e85590` accent color
- 16px gap

### Parameters

| Param | Min | Max | Step | Default |
|---|---|---|---|---|
| Drive | 0 | 1 | 0.01 | 0.40 |
| Intensity | 0 | 1 | 0.01 | 0.45 |
| Tone | 0 | 1 | 0.01 | 0.50 |
| Mix | 0 | 1 | 0.01 | 1.00 |

---

## 8. Output Module

### Layout
Vertical stack:
1. **Meter + Knob row** (horizontal: In meter | Output knob | Out meter)
2. **GR readout** (below)

### Meters
- **Width:** 6px each
- **Height:** 80px
- **Background:** `pluginBg`
- **Border:** 1px solid `pluginBorder`, 3px radius
- **Fill direction:** Bottom-up
- **Input meter fill:** `ombicBlue`
- **Output meter fill:** `ombicTeal`
- **Labels below:** 8px, weight 700, uppercase, `pluginMuted`
- **Gap between meter and knob:** 10px

### Output Knob
- 56px diameter, `ombicPurple` accent
- Standard knob label/value layout

### GR Readout
- Label: "GR", 8px, weight 700, uppercase, `pluginMuted`
- Value: 16px, weight 900, color-coded:
  - < 3 dB: `ombicTeal`
  - 3–6 dB: `ombicYellow`
  - > 6 dB: `ombicRed`
- Format: `-X.X dB`

### Parameters

| Param | Min | Max | Step | Default | Unit |
|---|---|---|---|---|---|
| Output | -24 | +24 | 0.1 | 0.0 | dB |

---

## 9. Header Bar

- **Background:** `ombicRed`
- **Height:** ~38px (10px vertical padding)
- **Left text:** "OMBIC COMPRESSOR", 18px, weight 900, uppercase, white, letter-spacing 1.5px
- **Right text:** "● Curve OK", 10px, weight 900, uppercase, white at 85% opacity

---

## 10. Footer Bar

- **Border top:** 1px solid `pluginBorder`
- **Padding:** 8px 20px
- **Three items spaced apart:** "Ombic Sound" | "v1.0.0" | "Stereo"
- **Font:** 9px, weight 700, uppercase, `pluginMuted`, letter-spacing 0.5px

---

## 11. Typography Reference

| Element | Font | Size | Weight | Color | Transform |
|---|---|---|---|---|---|
| Plugin title | Trash | 18px | 900 | white | uppercase |
| Module headers | Trash | 13px | 900 | white | uppercase |
| Param labels | Trash | 9px | 700 | `pluginMuted` | uppercase |
| Param values | Trash | 14px | 900 | `pluginText` | none |
| Large param value (Opto) | Trash | 20px | 900 | `pluginText` | none |
| Mode pills | Trash | 11px | 900 | varies | uppercase |
| Curve axis labels | Trash | 8px | 700 | 25% white | uppercase |
| Meter labels | Trash | 8px | 700 | `pluginMuted` | uppercase |
| GR label | Trash | 8px | 700 | `pluginMuted` | uppercase |
| GR value | Trash | 16px | 900 | color-coded | none |
| Footer | Trash | 9px | 700 | `pluginMuted` | uppercase |

---

## 12. Shadow System (Brand Signature)

Hard offset shadows, zero blur — this is the core Ombic visual identity.

- **Plugin container:** `8px 8px 0 0` in `pluginShadow`
- **No module-level shadows** (cards are flat within the plugin)
- **No soft/blurred shadows anywhere**

In JUCE, implement via `Graphics::setColour()` + `fillRoundedRectangle()` offset by 8px,8px behind the main plugin bounds, or use `DropShadow` with zero blur radius.

---

## 13. File Dependencies

```
fonts/
  Trash-Regular.ttf
  Trash-Bold.ttf
  Trash-Regular-Italic.ttf
  Trash-Bold-Italic.ttf

reference/
  ombic-compressor.html          ← Interactive mockup (open in browser)
  OMBIC_SOUND_STYLE_GUIDE.md     ← Brand system (web context)
  OMBIC_COMPRESSOR_JUCE_SPEC.md  ← This file
```
