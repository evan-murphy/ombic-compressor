# Ombic Compressor — Sidechain Filter Module Specification

> **For use with:** Cursor / AI code editors building the JUCE plugin.  
> **Parent document:** `OMBIC_COMPRESSOR_JUCE_SPEC.md` — this module follows all conventions defined there (color tokens, typography, knob rendering, shadow system, etc.).  
> **Signal flow position:** Input → **Sidechain Filter** → Compressor (Opto/FET/VCA) → Neon Saturation → Output

---

## 1. Module Purpose

The Sidechain Filter Module is an independent processing block that shapes the compressor's sidechain detector signal. It does **not** alter the audio path — it only affects what the compressor "listens to" when determining gain reduction.

Primary use cases:
- Preventing low-frequency content (kick drums, bass, rumble) from dominating gain reduction
- Enabling "pumpy" dance compression where the kick passes through largely uncompressed
- Cleaning up proximity effect or subsonic energy from triggering the detector
- Giving the user deliberate control over the compressor's frequency-weighted reactivity

---

## 2. Architecture

### Signal Routing

```
Audio Input ──┬──────────────────────────────► Audio Path (unmodified) ──► Compressor Audio In
              │
              └──► [Sidechain HPF] ──► Compressor Sidechain/Detector Input
```

- The audio path passes through this module untouched. No filtering, no latency, no coloration.
- A copy of the input signal is routed through the sidechain high-pass filter.
- The filtered copy is sent to whichever compressor topology is active (Opto/FET/VCA) as its detector input.
- When the filter frequency is at minimum (OFF), the sidechain receives an exact copy of the audio — equivalent to bypassing the module entirely.

### DSP Implementation

```cpp
// Filter type: 2nd-order Butterworth high-pass (12 dB/octave)
// This slope is standard for sidechain filtering — steep enough to be effective,
// gentle enough to avoid phase artifacts in the crossover region.

// Use juce::dsp::IIR::Filter or juce::dsp::ProcessorDuplicator
// with juce::dsp::IIR::Coefficients::makeHighPass()

// Coefficient update:
//   - Recalculate only when frequency parameter changes (not every sample)
//   - Use smoothed parameter values to avoid zipper noise during automation
//   - Sample rate aware — recalculate on prepareToPlay()

// Bypass behavior:
//   - When frequency param is at minimum position (OFF / 20 Hz), 
//     bypass the filter entirely (pass signal through unchanged)
//   - Do NOT process through a 20 Hz HPF when "off" — true bypass to avoid
//     any phase shift at the bottom of the range

// Processing:
//   - Process sidechain copy in-place
//   - Mono sum before filtering if stereo (standard sidechain behavior)
//   - Filter runs at full sample rate (no decimation)
```

### Listen/Audition Mode

When Listen mode is engaged:
- The plugin's audio output is temporarily replaced with the sidechain signal (post-filter)
- The compressor continues to operate normally — gain reduction does not change
- This lets the user hear exactly what the compressor is reacting to
- The Neon Saturation module's waveform scope should reflect the sidechain signal while Listen is active (if technically feasible without excessive coupling)

```cpp
// Listen mode implementation:
//   - Maintain a separate buffer of the filtered sidechain signal
//   - When listen == true, copy sidechain buffer to output buffer
//     AFTER the compressor has processed (so GR behavior is unaffected)
//   - Apply a short crossfade (1–5ms) when toggling listen on/off
//     to avoid clicks
//   - Listen output should be at unity gain (no makeup, no output knob applied)
//   - GR metering should still show actual compression behavior
```

---

## 3. Parameters

| Param | ID | Min | Max | Step | Default | Unit | Behavior |
|---|---|---|---|---|---|---|---|
| SC Frequency | `sc_frequency` | 20 | 500 | 1 | 20 (OFF) | Hz | Continuously variable. At 20 Hz, filter is bypassed (OFF). |
| Listen | `sc_listen` | 0 | 1 | 1 | 0 (off) | bool | Toggle. Auditions the sidechain signal. |

### Parameter Details

**SC Frequency:**
- Skewed/logarithmic taper — most of the knob travel should cover 20–200 Hz where the musically useful range lives. The 200–500 Hz range compresses into the upper portion of the sweep.
- Display format: When at minimum, display `OFF`. Otherwise display value as integer Hz (e.g., `80 Hz`, `250 Hz`).
- Automation-safe: smoothed value changes, no discontinuities.
- The 500 Hz ceiling is deliberate — going higher starts overlapping with the musical content range in ways that are confusing for users and better handled by a dedicated sidechain EQ plugin. 500 Hz is high enough to address boxy/muddy resonances if needed.

**Listen:**
- Momentary or latching behavior — implement as latching (toggle on click) but consider adding shift+click for momentary hold if JUCE supports this cleanly. If not, latching only is fine.
- Must be clearly indicated as active (see Visual Design below) — users must never accidentally leave this on during a mix.

---

## 4. Visual Design

### Module Card

Follows the standard module card conventions from the parent spec:
- Background: `pluginSurface` (`#181c2e`)
- Border: 2px solid `pluginBorder` (`#e6e9ef` at 20%)
- Border radius: 20px
- Padding: 16px

### Module Header

- Text: `SC FILTER`
- Color: `ombicTeal` (`#338fab`) — this module gets teal as its accent color, distinguishing it from the blue compressor, pink saturation, and purple output modules
- Font: Trash, 13px, weight 900, uppercase
- Background: follows the standard module header pattern from parent spec

### Accent Color

All interactive elements in this module use `ombicTeal` (`#338fab`):
- Knob arc fill
- Listen button active state
- Any active/hover highlights

### Layout

Vertical stack within the card:

```
┌─────────────────────────────────┐
│  SC FILTER                      │  ← Module header, ombicTeal
├─────────────────────────────────┤
│                                 │
│   ┌─────────────────────────┐   │
│   │   Frequency Response    │   │  ← Mini frequency response display
│   │   Visualization         │   │     (see below)
│   └─────────────────────────┘   │
│                                 │
│     ┌──────┐    ┌────────┐      │
│     │ FREQ │    │ LISTEN │      │  ← Knob + button, horizontally centered
│     │ knob │    │  btn   │      │
│     │80 Hz │    │        │      │
│     └──────┘    └────────┘      │
│                                 │
└─────────────────────────────────┘
```

### Frequency Response Visualization

A mini display showing the filter's frequency response curve — gives the user immediate visual feedback on what frequencies are being removed from the sidechain.

- **Size:** Full card width (minus padding) × 48px height
- **Background:** `pluginBg` (`#0f1220`)
- **Border:** 1px solid `pluginBorder`, 10px border radius
- **Margin bottom:** 14px (before controls)

**Rendering:**

```
Frequency axis (horizontal, log-scaled):
  20 Hz ────────────────────────────── 20 kHz
  
Amplitude axis (vertical, linear or dB):
  0 dB (top) ── unity passband
  -24 dB (bottom) ── max attenuation shown

Curve rendering:
  - Plot the HPF magnitude response from 20 Hz to 20 kHz
  - Stroke: ombicTeal, 1.5px solid
  - Glow: same path, 6px stroke, ombicTeal at 15% opacity
  - Fill below curve (rejected region): ombicTeal at 5% opacity
  - The filled region visually communicates "this energy is being removed 
    from the sidechain"

Grid:
  - Vertical lines at decade markers: 100 Hz, 1 kHz, 10 kHz
  - Style: 1px, 4% white (matches parent spec grid style)
  - Horizontal line at 0 dB: 1px, 4% white
  - Axis labels: 8px, weight 700, 25% white, uppercase
    Bottom: "20", "100", "1K", "10K", "20K"
    (No vertical axis labels needed — the display is too small)

When filter is OFF (frequency at minimum):
  - Show a flat line at 0 dB (full passband, no filtering)
  - Fill region disappears
  - Muted appearance — curve in pluginMuted at 40% opacity

Animation:
  - Curve updates smoothly when frequency knob is adjusted
  - Use ~60fps interpolation on the curve position (CSS transition 
    equivalent in JUCE: smooth the plot data, not just the DSP coefficients)
```

### SC Frequency Knob

- **Diameter:** 56px
- **Accent color:** `ombicTeal`
- **Arc sweep:** 240° (standard, matching parent spec knob system)
- **Knob center fill:** `pluginRaised` (`#222840`)
- **Track (inactive arc):** `0x14ffffff` (8% white)
- **Label:** "SC FREQ", 9px, weight 700, uppercase, `pluginMuted`, below knob
- **Value:** 14px, weight 900, `pluginText`, below label
  - At minimum: display `OFF` (not `20 Hz`)
  - Otherwise: `{value} Hz` (integer, no decimal)

### Listen Button

A small toggle button adjacent to the frequency knob (to its right, vertically centered with the knob).

- **Size:** 32px wide × 24px tall
- **Border radius:** 6px
- **Label:** "LISTEN" — 8px, weight 900, uppercase, centered in button

**Inactive state:**
- Background: `pluginBg`
- Border: 1.5px solid `pluginBorder`
- Text: `pluginMuted`

**Active state:**
- Background: `ombicTeal`
- Border: 1.5px solid `ombicTeal`
- Text: white
- Add a subtle pulse animation (opacity oscillates between 85%–100% on a 1.5s cycle) to ensure the user notices it's engaged. This is a safety feature — accidentally leaving Listen on during playback/export would be bad.

**Hover (inactive):**
- Border: `ombicTeal`
- Text: `pluginText`

### Listen-Active Global Indicator

When Listen mode is active, in addition to the button state, display a small indicator in the plugin header bar:

- Text: `● SC LISTEN` 
- Position: next to the existing "● Curve OK" status text
- Color: `ombicTeal`
- Font: 10px, weight 900, uppercase
- This ensures visibility even if the sidechain filter module is partially scrolled or in a collapsed state (future-proofing)

---

## 5. Module Placement in Plugin Layout

### Updated Signal Flow (left to right)

The plugin layout gains a new column. The updated grid structure:

```
┌──────────────────────────────────────────────────────────────────────────┐
│ Header bar                                                               │
├───────────┬──────────────────┬──────────────────┬────────────┬───────────┤
│           │                  │                  │            │           │
│ SC FILTER │   COMPRESSOR     │  NEON SATURATION │   OUTPUT   │           │
│           │                  │                  │            │           │
│  [freq    │  [curve display] │  [waveform scope]│  [meters]  │           │
│   resp]   │                  │                  │            │           │
│           │  [knobs]         │  [knobs]         │  [knob]    │           │
│  [knob]   │                  │                  │  [GR]      │           │
│  [listen] │                  │                  │            │           │
│           │                  │                  │            │           │
├───────────┴──────────────────┴──────────────────┴────────────┴───────────┤
│ Footer bar                                                               │
└──────────────────────────────────────────────────────────────────────────┘
```

### Grid Adjustment

The parent spec defines a 3-column grid. This becomes 4 columns:

```cpp
// Updated grid: SC Filter is a narrow first column
// Previous: Compressor (1.2fr) | Neon Saturation (1.3fr) | Output (0.7fr)
// Updated:  SC Filter (0.55fr) | Compressor (1.2fr) | Neon Saturation (1.3fr) | Output (0.7fr)

// SC Filter module minimum width: 120px
// SC Filter module preferred width: ~145px
// This is a deliberately compact module — it has fewer controls than the others
```

If this addition makes the plugin feel too wide at 900px, consider:
- Increasing default width to 960px or 980px
- Or slightly reducing the compressor/saturation column fractions to accommodate

The SC Filter module should feel like a utility block — present and accessible but not competing with the compressor and saturation modules for visual attention.

---

## 6. Interaction Behavior

### Knob Interaction

Follows the parent spec's knob interaction model:
- Vertical drag to adjust value
- Double-click to reset to default (20 Hz / OFF)
- Right-click for value entry (type exact Hz value)
- Mouse wheel for fine adjustment
- Shift+drag for fine-grained control

### Listen Button Interaction

- Single click: toggle on/off
- The button should feel immediately responsive — no animation delay on state change
- If implementing momentary mode: Shift+click = hold while pressed, release to deactivate

### Keyboard Shortcuts (optional, nice-to-have)

- If the plugin implements keyboard focus: `L` key toggles Listen mode when the SC Filter module has focus

---

## 7. State Persistence

- `sc_frequency` and `sc_listen` must be saved/restored with the plugin state (via `getStateInformation` / `setStateInformation`)
- **Important:** `sc_listen` should default to OFF on session load regardless of saved state. This is a safety measure — you never want a session to open with the sidechain audition active, as it would replace the audio output with the filtered detector signal. Override the saved value to `false` on load.

---

## 8. Automation

- `sc_frequency` is fully automatable. Use parameter smoothing (10–20ms ramp time) to prevent clicks/zipper noise during automated sweeps.
- `sc_listen` is automatable but generally shouldn't be — it's a monitoring utility. If your DAW exposes it, that's fine, but don't prioritize smooth automation of a boolean toggle.

---

## 9. JUCE Implementation Notes

### Recommended Classes

```cpp
// DSP
#include <juce_dsp/juce_dsp.h>

// Sidechain filter: use ProcessorDuplicator for stereo
juce::dsp::ProcessorDuplicator<
    juce::dsp::IIR::Filter<float>,
    juce::dsp::IIR::Coefficients<float>
> sidechainHPF;

// Coefficient generation
auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(
    sampleRate,
    frequencyHz,
    0.7071f  // Q = 1/√2, Butterworth alignment
);

// Parameter smoothing
juce::SmoothedValue<float> smoothedFrequency;
smoothedFrequency.reset(sampleRate, 0.015);  // 15ms ramp
```

### Processing Architecture

```cpp
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    // 1. Copy input to sidechain buffer
    sidechainBuffer.makeCopyOf(buffer);
    
    // 2. If filter is not OFF, process sidechain buffer through HPF
    if (currentFrequency > 20.0f)  // 20 Hz = OFF position
    {
        // Update coefficients if frequency changed
        if (smoothedFrequency.isSmoothing())
        {
            float freq = smoothedFrequency.getNextValue();
            *sidechainHPF.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
                sampleRate, freq, 0.7071f
            );
        }
        
        // Process sidechain copy
        juce::dsp::AudioBlock<float> scBlock(sidechainBuffer);
        juce::dsp::ProcessContextReplacing<float> scContext(scBlock);
        sidechainHPF.process(scContext);
    }
    
    // 3. Pass sidechainBuffer to compressor's detector input
    //    (compressor module reads from this buffer for its envelope follower)
    compressorModule.setSidechainInput(sidechainBuffer);
    
    // 4. Process audio through compressor (uses original buffer for audio path)
    compressorModule.processBlock(buffer, midiMessages);
    
    // 5. If listen mode is active, replace output with sidechain signal
    if (listenMode)
    {
        // Crossfade logic for click-free switching would go here
        buffer.makeCopyOf(sidechainBuffer);
    }
    
    // 6. Continue to Neon Saturation, Output, etc.
}
```

### Parameter Registration

```cpp
// In createParameterLayout():
layout.add(std::make_unique<juce::AudioParameterFloat>(
    "sc_frequency",                          // parameterID
    "SC Frequency",                          // parameter name
    juce::NormalisableRange<float>(
        20.0f, 500.0f,                       // min, max
        1.0f,                                // step
        0.35f                                // skew: log-ish taper, 
                                             // concentrates travel in 20–200 Hz
    ),
    20.0f,                                   // default (OFF)
    "Hz",                                    // label
    juce::AudioProcessorParameter::genericParameter,
    [](float value, int) {                   // valueToText
        if (value <= 20.0f) return juce::String("OFF");
        return juce::String(juce::roundToInt(value)) + " Hz";
    },
    [](const juce::String& text) {           // textToValue
        if (text.containsIgnoreCase("off")) return 20.0f;
        return text.getFloatValue();
    }
));

layout.add(std::make_unique<juce::AudioParameterBool>(
    "sc_listen",                             // parameterID
    "SC Listen",                             // parameter name
    false                                    // default
));
```

### GUI Component

```cpp
class SidechainFilterModule : public juce::Component, public juce::Timer
{
public:
    // Contains:
    //   - FrequencyResponseDisplay (custom Component, 48px tall)
    //   - RotarySlider for sc_frequency (56px, ombicTeal accent)
    //   - ListenButton (custom TextButton subclass, 32x24px)
    //   - Labels
    
    // FrequencyResponseDisplay:
    //   - Renders HPF magnitude response via juce::dsp::IIR::Coefficients
    //   - Call repaint() when frequency parameter changes
    //   - Use Path for the curve, fill below with ombicTeal at 5% alpha
    //   - Log-scaled x-axis: 20 Hz to 20 kHz
    //   - Linear or dB y-axis: 0 to -24 dB
    
    // Timer callback at ~60fps for smooth curve animation during knob drags
    void timerCallback() override
    {
        if (frequencyDisplay.needsRepaint())
            frequencyDisplay.repaint();
    }
};
```

---

## 10. Testing Checklist

### DSP Verification
- [ ] With SC Freq at OFF (20 Hz): sidechain signal is identical to input (true bypass, no phase shift)
- [ ] With SC Freq at 100 Hz: confirm 12 dB/octave rolloff below 100 Hz on sidechain signal
- [ ] Sweeping SC Freq during playback: no clicks, zipper noise, or discontinuities
- [ ] Listen mode: output matches sidechain signal, gain reduction behavior unchanged
- [ ] Listen mode toggle: click-free crossfade on engage/disengage
- [ ] Mono sum: sidechain correctly sums stereo input to mono before filtering
- [ ] Sample rate change: filter coefficients recalculated correctly at 44.1k, 48k, 88.2k, 96k, 176.4k, 192k

### Functional Verification
- [ ] Low-frequency test: 40 Hz sine + 4 kHz sine, SC Freq at 80 Hz — compressor should react primarily to the 4 kHz tone, not the 40 Hz
- [ ] Dance music test: full mix with prominent kick, SC Freq at ~150 Hz — kick should pass through with minimal gain reduction, other elements compressed normally
- [ ] Proximity effect test: vocal with bass buildup, SC Freq at ~100 Hz — compression should respond to vocal presence, not proximity rumble

### UI Verification
- [ ] Frequency response display updates in real-time during knob adjustment
- [ ] Display shows flat line when filter is OFF
- [ ] Knob displays "OFF" at minimum, Hz values otherwise
- [ ] Listen button shows active state clearly (teal fill, pulse animation)
- [ ] Header bar shows "● SC LISTEN" indicator when listen is active
- [ ] Double-click knob resets to OFF
- [ ] Module renders correctly at minimum width (120px) and preferred width (145px)
- [ ] sc_listen resets to OFF on session load (safety override)

### Integration Verification
- [ ] Compressor reacts to filtered sidechain, not raw input, when filter is active
- [ ] All three compressor topologies (Opto/FET/VCA) correctly receive the sidechain signal
- [ ] Switching compressor modes does not reset or bypass the sidechain filter
- [ ] Plugin state save/restore correctly preserves sc_frequency value
- [ ] Plugin state restore correctly overrides sc_listen to false

---

## 11. Future Considerations (Out of Scope for v1)

These are not part of the initial implementation but the architecture should not prevent them:

- **External sidechain input:** Accepting a sidechain signal from the DAW (standard in most compressor plugins). The filter would then process the external signal instead of a copy of the main input.
- **LPF option:** Adding a low-pass filter for de-essing applications. Would require a second knob or a mode toggle. Currently out of scope — keep the module focused.
- **Filter slope selector:** Switching between 6 dB/oct, 12 dB/oct, 18 dB/oct, 24 dB/oct. Useful for surgical work but adds complexity. The 12 dB/oct Butterworth default covers 90%+ of use cases.
- **Sidechain from post-saturation:** Routing the sidechain tap from after the Neon Saturation module for feedback-style compression behavior. Architecturally interesting but niche.
