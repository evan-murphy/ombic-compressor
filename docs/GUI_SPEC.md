# Compressor + Neon Saturator Plugin — GUI Specification

This document defines the component architecture, parameter mapping, mode switching, signal-flow visualization, and metering for the VST3 compressor plugin with neon saturation, aligned with the OMBIC Sound style guide.

---

## 1. Component Architecture

### 1.1 Hierarchy

```
PluginEditor (AudioProcessorEditor)
├── MainContainer (Component)
│   ├── HeaderBar (Component)           — "OMBIC" / plugin name, red bar
│   ├── SignalFlowStrip (Component)      — [IN] ⇄ Saturator ⇄ Compressor ⇄ [OUT]
│   ├── ContentArea (Component)
│   │   ├── CompressorSection (Component)
│   │   │   ├── ModeSelector (ComboBox or ToggleButtons)  — Opto / FET / …
│   │   │   ├── CompressorControls (Component)            — threshold, ratio, attack, release
│   │   │   ├── OutputSection (Component)                 — output gain (makeup/trim)
│   │   │   └── GainReductionMeter (Component)            — GR meter
│   │   ├── SaturatorSection (Component)
│   │   │   ├── SaturatorControls (Component)             — drive, tone, mix (+ depth, bandwidth, dry_wet)
│   │   │   └── Optional: SaturatorOnOff (ToggleButton)
│   │   └── MeterStrip (Component)
│   │       ├── InputLevelMeter (Component)
│   │       ├── GainReductionMeter (Component)
│   │       └── OutputLevelMeter (Component)
│   └── Footer (optional — bypass, preset name)
```

### 1.2 JUCE Component Mapping

| Logical element           | JUCE type              | Notes |
|---------------------------|------------------------|--------|
| PluginEditor              | `AudioProcessorEditor` | Owns `APVTS&`, attachments, timer for meters |
| MainContainer             | `Component`            | Layout: FlexBox or Grid |
| HeaderBar                 | `Component` (custom paint) | OMBIC red bg, white text |
| SignalFlowStrip           | `Component`            | Paints chain + routing toggle; contains `SignalFlowSwitcher` |
| SignalFlowSwitcher        | `ToggleButton` or custom | "Saturator: Before / After" or A/B buttons |
| CompressorSection         | `Component`            | Card style (white, 3px border, hard shadow) |
| ModeSelector              | `ComboBox`             | Opto, FET (extensible) |
| CompressorControls        | `Component`            | Contains `Slider` + `Label` for each param |
| SaturatorSection          | `Component`            | Same card style |
| SaturatorControls         | `Component`            | Drive, Tone, Mix sliders |
| MeterStrip                | `Component`            | Horizontal strip of meter components |
| InputLevelMeter           | `Component` (custom paint) | Vertical bar, thread-safe level from processor |
| GainReductionMeter        | `Component` (custom paint) | GR in dB, from processor |
| OutputLevelMeter           | `Component` (custom paint) | Same as input |

### 1.3 Style (OMBIC)

- **Cards**: `background = white`, `border = 3px solid #0f1220`, `borderRadius = 20px`, `shadow = 8px 8px 0 0 #0f1220`.
- **Header**: background `#ff001f` (ombic-red), text white, font weight 900.
- **Section headers**: background `#076dc3` (ombic-blue), text white.
- **Primary buttons / selected**: blue; **Selected card state**: yellow `#ffce00`.
- **Font**: Trash if available in `style/` (e.g. Trash.ttf), else system; weight ≥ 700.
- **LookAndFeel**: Custom `OmbicLookAndFeel` overriding `drawSlider()`, `drawComboBox()`, `drawButton()`, etc.

---

## 2. Parameter Mapping

### 2.1 AudioProcessorValueTreeState (APVTS) Parameter IDs

All parameters are registered with `AudioProcessorValueTreeState` and attached to the GUI via `SliderAttachment`, `ComboBoxAttachment`, `ButtonAttachment`.

#### Compressor

| Parameter ID       | Type    | Range / Options      | Default | Notes |
|--------------------|---------|----------------------|---------|--------|
| `compressor_mode`  | Choice  | 0 = Opto, 1 = FET | 0       | ComboBoxAttachment; drives visibility of ratio/attack/release |
| `threshold`        | Float   | 0…100 (Opto) or -60…0 dB (FET) | 50 / -18 | SliderAttachment; scale depends on mode |
| `ratio`            | Float   | 1…20 (FET only)   | 4       | SliderAttachment; hidden in Opto |
| `attack`           | Float   | 0.02…0.8 ms (FET) or N/A | 0.1 | SliderAttachment; hidden in Opto |
| `release`          | Float   | 50…1100 ms (FET)  | 200     | SliderAttachment; hidden in Opto |
| `opto_compress_limit` | Choice | 0 = Compress, 1 = Limit | 0 | ComboBoxAttachment; **Opto only**; visible when mode = Opto |
| `makeup_gain_db`   | Float   | -24…+12 dB           | 0       | SliderAttachment; **Output** section (after saturator + compressor); boost capped for safe listening |

#### Saturator (Neon)

| Parameter ID            | Type  | Range     | Default | Notes |
|--------------------------|-------|-----------|---------|--------|
| `saturator_position`     | Bool  | 0 = after, 1 = before compressor | 0 | ButtonAttachment or ComboBoxAttachment |
| `neon_enable`            | Bool  | 0/1       | 0       | ToggleButtonAttachment |
| `neon_drive`             | Float | 0…1 (maps to depth 0…0.05) | 0.4 | SliderAttachment |
| `neon_tone`              | Float | 0…1 (maps to 200…5000 Hz) | 0.5 | SliderAttachment → modulation_bandwidth_hz |
| `neon_mix`               | Float | 0…1 (dry/wet) | 1.0 | SliderAttachment |
| `neon_burstiness`        | Float | 0…10      | 0       | SliderAttachment (optional in GUI) |
| `neon_g_min`             | Float | 0.85…1.0  | 0.92    | SliderAttachment (optional) |
| `neon_saturation_after`   | Bool  | 0/1 (soft sat after multiply) | 0 | ToggleButton (optional) |

#### Metering (read-only from processor)

- Input level (dB or linear), gain reduction (dB), output level (dB) are **not** APVTS parameters; they are written by the processor into atomic/float variables and read on the message thread in the editor’s `timerCallback()` for painting.

### 2.2 Attachments in PluginEditor

- **Constructor**: Create all controls, then create attachments using `APVTS&` and parameter IDs. Store attachments as member `std::unique_ptr<SliderAttachment>` etc., so they outlive the controls and disconnect automatically.
- **SliderAttachment**: `sliderId` must match the parameter ID string in `createParameterLayout()`.
- **ComboBoxAttachment**: for `compressor_mode`, `opto_compress_limit` (Opto), and optionally `saturator_position` (if using ComboBox).
- **ButtonAttachment**: for `neon_enable`, `saturator_position` (if using toggle), `neon_saturation_after`.

---

## 3. Mode Switching Implementation

### 3.1 Compressor Mode (Opto vs FET)

- **Control**: `ComboBox` with items "Opto" (index 0), "FET" (index 1). Parameter `compressor_mode` (choice 0/1).
- **Behaviour**:
  - When mode = Opto: show **Threshold** (0–100%) and **Compress / Limit** (ComboBox). Hide **Ratio**, **Attack**, **Release**.
  - When mode = FET: show **Threshold** (dB), **Ratio**, **Attack**, **Release**. Hide **Compress / Limit**.
  - **Output** (makeup/trim) is a separate section after Saturator, not part of the Compressor card.
- **Implementation**:
  - In `PluginEditor::timerCallback()` or in a `ComboBox::onChange` callback (if not using attachment), update visibility/enabled state of the FET-only controls from `compressor_mode` value.
  - Alternatively use a single `ComboBoxAttachment` for `compressor_mode` and in `valueChanged()` of a listener on the APVTS, update visibility. Prefer one listener on the APVTS parameter `compressor_mode` to show/hide components.

### 3.2 Parameter Ranges per Mode

- **Opto**: `threshold` 0…100 (percent); ratio/attack/release unused by DSP.
- **FET**: `threshold` in dB (e.g. -60…0), `ratio` 1…20, `attack` in ms or µs per config, `release` in ms.
- Store one set of parameter IDs; in the processor’s `processBlock()`, read mode and threshold (and ratio/attack/release for FET) and pass to the appropriate curve or emulation.

---

## 4. Signal Flow Visualization

### 4.1 Layout

A horizontal strip showing the chain with the saturator position switchable:

```
[ IN ]  ———  [ Saturator ]  ———  [ Compressor ]  ———  [ OUT ]
         or
[ IN ]  ———  [ Compressor ]  ———  [ Saturator ]  ———  [ OUT ]
```

### 4.2 Interactive Control

- **Option A**: Two buttons "Before" / "After" (or "Pre" / "Post"). Only one active; sets `saturator_position` (e.g. 1 = before, 0 = after).
- **Option B**: A single toggle button that flips between "Saturator → Compressor" and "Compressor → Saturator", with the strip graphic updating (e.g. swap order of two drawn blocks).
- **Implementation**:
  - `SignalFlowStrip` owns a small `Component` that paints the blocks and arrows. It also owns the switcher (e.g. `ToggleButton` "Before" / "After" or a custom button that cycles).
  - When `saturator_position` changes (via attachment or button callback), call `repaint()` on the strip so the drawn order matches.
  - In the strip’s `paint()`, read `saturator_position` from APVTS (or a cached value) and draw either:
    - IN → Saturator → Compressor → OUT, or
    - IN → Compressor → Saturator → OUT.

### 4.3 GUI Code Structure

- `SignalFlowStrip : public Component`  
  - Overrides `paint(Graphics&)` to draw boxes and arrows.  
  - Adds child `SignalFlowSwitcher` (e.g. `ToggleButton` or two-text-button component).  
  - Gets `saturator_position` from processor or from a `Label`/attachment passed in (or store a reference to APVTS and read parameter).
- `SignalFlowSwitcher`: Buttons or ComboBox that update `saturator_position` via APVTS (attachment or explicit `setParameterNotifyingHost()`).

---

## 5. Metering and Feedback

### 5.1 Meters Required

| Meter                | Quantity | Description |
|----------------------|----------|-------------|
| Input level          | 1 (or L/R) | RMS or peak of input buffer, dB |
| Gain reduction      | 1        | Current GR from compressor (dB) |
| Output level        | 1 (or L/R) | RMS or peak of output buffer, dB |

### 5.2 Thread Safety

- **Audio thread**: Processor computes input level, GR, output level per block and writes into `std::atomic<float>` or a lock-free level holder (e.g. one float per meter, updated with relaxed store).
- **Message thread**: Editor’s `startTimerHz(30)` (or 24) in constructor; in `timerCallback()` read the atomics and update member variables used in `paint()`. Call `repaint()` on the meter components so they redraw with the new values.
- **Smoothing**: Optional one-pole smoothing on the message thread for display (e.g. `levelSmoothed += 0.3f * (levelNew - levelSmoothed)`).

### 5.3 Implementation Details

- **Processor**: Expose `getInputLevelDb()`, `getGainReductionDb()`, `getOutputLevelDb()` (or equivalent) returning the last written atomic values.
- **Meter component**: In `paint()`, draw a vertical (or horizontal) bar; height/width proportional to `juce::jmap(levelDb, -60.0f, 0.0f, 0.0f, 1.0f)` (or similar range). Use OMBIC colours (e.g. blue for level, red for GR).
- **Gain reduction meter**: Often drawn “inverted” (bar goes down as GR increases). Scale: 0 dB at top, e.g. -20 dB at bottom; bar length = min(GR, 20) dB.
- **Efficiency**: Only repaint the meter strip (or the whole editor) when levels change; avoid repainting the entire editor at 30 Hz if only meters change by using a small `MeterStrip` that you call `repaint()` on from `timerCallback()`.

---

## 6. Summary Checklist

- [ ] All compressor and saturator parameters defined in `createParameterLayout()` with correct IDs.
- [ ] SliderAttachment / ComboBoxAttachment / ButtonAttachment for every control in PluginEditor.
- [ ] Mode selector drives visibility/enabled state of Opto vs FET controls.
- [x] Signal flow fixed: saturator always before compressor (no strip/toggle).
- [ ] Processor writes meter values to atomics; editor timer reads and repaints meter components.
- [ ] OMBIC LookAndFeel applied: borders, shadows, colours, fonts per style guide.
