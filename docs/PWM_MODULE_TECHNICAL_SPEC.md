# Ombic Compressor — PWM Module Technical Specification

**Version:** 1.1  
**Date:** February 17, 2026  
**Author:** Evan / Claude (Anthropic)  
**Status:** Draft — Specification  
**Scope:** New PWM compressor mode + shared Iron transformer saturation stage

---

## 1. Executive Summary

This specification defines two additions to the Ombic Compressor plugin:

1. **PWM Compressor Mode** — A third compressor topology (alongside Opto and FET) that models the behavior of vintage pulse-width-modulation compressors, particularly the Pye 4060 family. The PWM mode's defining characteristic is *transparent gain reduction* with character derived from the surrounding analog circuit, not the gain element itself.

2. **Iron (Transformer Saturation) Stage** — A new shared processing block in the Output module, available to all compressor modes. Models the deterministic, signal-dependent harmonic coloration of audio output transformers.

### Design Philosophy

Extensive research (documented in project Perplexity research files) establishes that:

- Ideal PWM gain reduction is mathematically equivalent to sample-by-sample digital multiplication. There is nothing inherently unique about the PWM switching process to model in DSP.
- The sonic identity of real PWM compressors (Pye 4060, Crane Song STC-8) comes from: (a) the sidechain/detector topology, (b) transformer saturation, (c) output stage character, and (d) the *absence* of gain-element distortion that other topologies introduce.
- No existing plugin actually simulates PWM carrier switching. All PWM-labeled plugins model the surrounding analog behaviors.

Therefore, the PWM mode is designed around what makes PWM compressors *sound and behave differently* — not around simulating a chopper circuit.

---

## 2. Signal Flow Integration

### Current Architecture

```
Input → [Sidechain: mono sum → optional HPF 20-500Hz] → Neon Saturator → Compressor (FET|Opto) → Output
```

### Updated Architecture

```
Input → [Sidechain: mono sum → optional HPF 20-500Hz] → Neon Saturator → Compressor (FET|Opto|PWM) → Iron → Output
```

Changes:

- **Compressor block** gains a third mode: PWM. Implemented as a new `pwmChain_` alongside existing `fetChain_` and `optoChain_`. Only one runs per process call.
- **Iron stage** is inserted between the Compressor output and the Output gain/trim. It is always in the signal path (controlled by an Iron amount parameter; 0% = bypass/clean passthrough). Iron sits *after* compression and *before* makeup gain.
- **SC Listen behavior** is unchanged. When SC Listen is on, the main output is replaced by the sidechain signal at unity; Iron and Output gain are bypassed.

### Processing Order (per sample block)

```
1. Input metering (RMS)
2. Sidechain derivation: mono sum of input
3. Sidechain HPF (if SC Frequency > 20 Hz)
4. Neon Saturator (Drive, Tone, Mix, Intensity, Burstiness, G Min, Soft Sat After)
5. Compressor: one of FET, Opto, or PWM
   - Detector source: external sidechain (if SC Freq > 20 Hz) OR internal (mode-dependent)
   - Gain reduction applied to main stereo buffer
6. Iron transformer saturation (Iron amount; mode-aware character)
7. Output: if Auto Gain on, add parameter-based makeup; then apply Output gain/trim (if SC Listen is off)
   OR SC Listen output (if SC Listen is on — bypasses steps 6-7)
```

---

## 3. PWM Compressor Mode

### 3.1 Overview

The PWM mode models a feedback-topology compressor with a clean gain element, inspired by the Pye 4060 family. Its defining sonic characteristics are:

- **Ultra-clean gain reduction** — no VCA leakage, no FET variable-resistance distortion, no opto nonlinearity. The gain change itself is transparent.
- **Fast, decisive envelope** — capable of very fast attack with program-dependent release.
- **Feedback topology** — compression is controlled by the output signal, not the input. This creates a softer, more musical knee than feedforward designs and introduces natural program-dependency.
- **Aggressive internal sidechain HPF** — when no external sidechain is active, the PWM detector has a built-in high-pass that "cuts body, preserves transients" (the Pye signature).

### 3.2 User Controls

The PWM mode exposes three controls on the Compressor Module:

| Parameter | Range | Step | Default | Unit | Notes |
|-----------|-------|------|---------|------|-------|
| Threshold | 0–100 | 0.1 | 50.0 | % | Shared control. Mapped internally to feedback-topology equivalent. See §3.4. |
| Ratio | 1.5–8.0 | 0.1 | 3.0 | :1 | Constrained range. PWM compressors are bus glue, not brickwall limiters. |
| Speed | 0–100 | 1 | 40 | % | Single combined attack/release control. See §3.5. |

**What is intentionally absent:**

- No separate Attack and Release — collapsed into Speed with program-dependent release. This reflects real Pye hardware (switchable speed settings, not independent A/R) and is the primary UX differentiator from FET mode.
- No Compress/Limit switch — that is Opto-specific behavior tied to its internal sidechain shelf.

### 3.3 Mode Selector Integration

The mode switcher pill row updates to:

```
[ Opto ] [ FET ] [ PWM ]
```

PWM pill follows the same styling convention as Opto and FET (active: fill + border + white text; inactive: transparent + border + muted text).

**Accent color for PWM mode:** `ombicTeal` (#00a67e). This differentiates PWM visually from Opto and FET (both `ombicBlue`) and communicates "clean, precise, technical." The transfer curve, knob arcs, and any PWM-specific UI elements use teal.

### 3.4 Gain Reduction — Feedback Topology

The PWM compressor uses a **feedback** detector topology, meaning the detector reads the signal *after* gain reduction has been applied. This is the same fundamental topology as the Pye 4060 and is distinct from FET mode (which reads from the main buffer or external detector in a more feedforward fashion).

**Feedback compression behavior:**

```
detector_input = output_of_gain_stage (post-GR)
envelope = smooth(detector_input, attack_time, release_time)
gain_reduction_dB = compute_gr(envelope, threshold, ratio)
output = input * dB_to_linear(gain_reduction_dB)
```

Because the detector reads its own output, the system is self-regulating:

- Actual compression ratio is softer than the set ratio (the feedback loop reduces apparent ratio).
- The knee is inherently soft/musical without explicit knee modeling.
- Program-dependent behavior emerges naturally — louder signals get compressed harder, which reduces the detector input, which eases compression, creating a dynamic equilibrium.

**Threshold mapping:**

The 0–100% Threshold control maps to the PWM feedback detector's internal reference level. At 0% = maximum sensitivity (compresses everything), at 100% = minimum sensitivity (only compresses the loudest signals). The internal mapping should be calibrated so that typical program material begins compressing around 40–60% threshold with sensible gain reduction amounts (2–6 dB).

**Ratio behavior:**

The set ratio (1.5:1 to 8:1) is the *nominal* ratio. Due to the feedback topology, the *effective* ratio at the output is always somewhat lower than the set value. This is a feature, not a bug — it's the "musical" quality of feedback compressors. At a set ratio of 4:1, the effective ratio may be closer to 2.5–3:1 depending on program material.

### 3.5 Speed Control

The Speed knob replaces separate Attack/Release controls with a single parameter that moves along a linked curve:

| Speed % | Attack (ms) | Release base (ms) | Character |
|---------|-------------|-------------------|-----------|
| 0 (slowest) | 80 | 800 | Transparent bus glue. Minimal pumping. |
| 25 | 30 | 400 | Smooth leveling. Gentle grab. |
| 50 (default area) | 10 | 200 | Balanced. Musical compression. |
| 75 | 2 | 80 | Aggressive grab. Pye-style "cut the body." |
| 100 (fastest) | 0.5 | 30 | Extreme. Limiter-like clamping on transients. |

**Attack curve:** Logarithmic mapping. Most of the perceptual change happens in the 0–50% range (80ms → 10ms). The 50–100% range covers the fast/extreme territory (10ms → 0.5ms).

**Release curve:** Also logarithmic, but with **program-dependent modulation**. The base release time from the table is the *minimum* release. The actual release time is extended dynamically based on the depth and duration of gain reduction:

```
actual_release = base_release * (1.0 + program_dependent_factor)

where:
  program_dependent_factor = gr_depth_dB * hold_duration_seconds * k
  k = tuning constant (start with 0.3, adjust by ear)
```

This means:

- Short transient peaks: release at the base time (fast recovery).
- Sustained loud passages: release slows down automatically (avoids pumping).
- This is the core Pye behavior — the compressor "breathes" with the program material.

### 3.6 Internal Sidechain (PWM-Specific)

Following the existing pattern where Opto has its own internal sidechain behavior when no external detector is active:

**When SC Frequency = 20 Hz (bypass / no external sidechain):**

The PWM mode engages its internal detector sidechain:

- **HPF at 150 Hz** — 2nd-order Butterworth (Q = 0.7071), same filter type as the shared SC Filter but at a fixed frequency. This is the Pye signature: the compressor doesn't react to low-frequency content, so bass passes through relatively uncompressed while midrange and upper-frequency energy drives the gain reduction. The result is "cuts body, preserves transients and low-end weight."
- **No additional HF shelf** — unlike Opto's Limit mode, the PWM internal sidechain does not boost HF sensitivity. The internal HPF alone produces the characteristic PWM/Pye behavior.

**When SC Frequency > 20 Hz (external sidechain active):**

The PWM internal sidechain is overridden. The external sidechain (mono sum → HPF at user-set frequency) feeds the PWM detector directly. This is identical to how Opto behaves — the shared sidechain replaces the internal one.

**Implementation note:** The internal 150 Hz HPF should be a separate filter instance from the shared sidechain filter. It only runs when the shared SC is bypassed. When the shared SC is active, the PWM internal filter is not processed.

### 3.7 Transfer Curve Display

The PWM transfer curve should look visually distinct from FET:

- **Soft knee:** The feedback topology produces an inherently soft knee. Rather than a sharp inflection at the threshold point, the curve should show a gradual transition. The visual knee width should be approximately ±3 dB around the threshold.
- **Color:** `ombicTeal` for the curve line and glow, replacing `ombicBlue`.
- **Glow:** Same rendering approach as FET (2px solid line + 8px glow at 20% opacity), but in teal.
- **Dynamic ratio indication:** Because the feedback topology means the effective ratio is lower than the set ratio, consider rendering a subtle secondary "effective curve" in a dimmer shade behind the nominal curve. This communicates the feedback behavior visually. (Optional — evaluate in prototype.)

### 3.8 Knob Layout

Three knobs in a horizontal row: **Threshold**, **Ratio**, **Speed**.

- All 60px diameter (same as FET knobs).
- 16px gap between knobs.
- Knob arc accent: `ombicTeal` (#00a67e).
- Labels above (9px, weight 700, uppercase, pluginMuted).
- Values below (14px, weight 900, pluginText).
- Minimum card width: ~260px (three 60px knobs + two 16px gaps + padding). Narrower than FET's four-knob layout.

**Grid layout when PWM is active:**

The compressor module is narrower than FET but wider than Opto. Suggested proportions:

```
Compressor: Fr(75)  |  Neon Saturation: Fr(160)  |  Output: Fr(75)
```

(Slightly narrower compressor than Opto's Fr(85) since we have three small knobs vs one large knob, but adjust based on visual testing.)

---

## 4. Iron — Transformer Saturation Stage

### 4.1 Overview

Iron is a new shared processing block that models audio output transformer saturation. It sits between the Compressor and the Output gain in the signal path. It is available to all compressor modes.

Iron is *not* the same thing as Neon:

| | Neon Saturator | Iron |
|---|---|---|
| **What it models** | Neon bulb gas-discharge stochastic modulation | Transformer core electromagnetic saturation |
| **Character** | Random, alive, movement — tape-like micro-variation | Deterministic, signal-dependent — warmth and weight |
| **Position** | Before compressor | After compressor |
| **Primary effect** | Adds texture (stochastic gain modulation) | Adds color (harmonic distortion + frequency shaping) |
| **Frequency behavior** | Modulation bandwidth controlled by Tone | Saturation stronger at LF (core flux ∝ 1/f) |
| **Harmonic signature** | Noise-like, broadband | Low-order harmonics (2nd dominant, 3rd secondary) |

### 4.2 User Control

A single control added to the Output Module:

| Parameter | Range | Step | Default | Unit | Notes |
|-----------|-------|------|---------|------|-------|
| Iron | 0–100 | 1 | 0 | % | Transformer saturation intensity. 0 = clean bypass. 100 = full vintage iron. |

**Accent color:** `ombicYellow` (#e5a800) — communicates warmth. The Iron knob arc uses yellow, distinct from the Output knob's `ombicPurple`.

**Default of 0%:** Iron is off by default. This ensures the plugin sounds "clean" out of the box and the user opts in to transformer coloration. This is the conservative, professional default — engineers expect a compressor to be transparent until they add color.

### 4.3 DSP Behavior

The Iron stage models three perceptually important characteristics of audio transformer saturation, identified by research (Huddersfield perceptual evaluation, AES 2021–2024 literature):

#### 4.3.1 Harmonic Generation (Highest Perceptual Priority)

Soft-clipping waveshaper that generates primarily 2nd-order harmonics, with 3rd-order secondary:

- **2nd harmonic** dominates at mild-to-moderate Iron settings (0–60%). This is the "warmth" component. Generated via asymmetric soft clipping.
- **3rd harmonic** rises at higher settings (60–100%). This adds "density" and mild grit.
- **Higher orders** (5th+) remain negligible unless Iron is near maximum. These are masked in typical program material and are not perceptually important per research.

**Implementation approach:** Asymmetric tanh-family waveshaper with Iron-dependent drive:

```
// Pseudocode — asymmetric soft clip for even harmonic generation
drive = iron_amount * max_drive;  // iron_amount: 0.0–1.0, max_drive: tune by ear (start ~3.0)
asymmetry = 0.15 * iron_amount;   // slight positive-peak bias for even harmonics

x_driven = x * (1.0 + drive);
x_biased = x_driven + asymmetry;
y = tanh(x_biased) - tanh(asymmetry);  // DC-corrected asymmetric saturation
output = mix(x, y, iron_amount);        // crossfade from clean to saturated
```

**Oversampling:** The waveshaper should run at 2x–4x oversampling to avoid aliasing from harmonic generation. Downsample with a linear-phase anti-alias filter after processing.

#### 4.3.2 Frequency-Dependent Saturation (Medium Perceptual Priority)

Transformer core flux is proportional to 1/f, meaning low frequencies saturate the core earlier and harder than high frequencies. This produces:

- **LF thickening:** +1–3 dB of low-order harmonic energy below ~200 Hz at moderate Iron settings. This is the "fullness" or "weight" that transformers add.
- **Midrange remains relatively linear** until driven very hard.
- **HF rolloff:** Gentle high-frequency attenuation above ~8–12 kHz from interwinding capacitance and core losses. This is the "smoothing" effect.

**Implementation approach:** Frequency-dependent drive. Apply more waveshaper drive to LF content than HF content:

```
// Split or weight by frequency before the waveshaper
lf_boost = low_shelf_filter(x, freq=200Hz, gain=+iron_amount * 4.0dB)
hf_cut = high_shelf_filter(x, freq=10kHz, gain=-iron_amount * 2.5dB)

// Or: use a pre-emphasis / de-emphasis pair around the waveshaper
pre_emphasized = apply_lf_boost(x)
saturated = waveshaper(pre_emphasized)
output = apply_lf_cut(saturated)  // restore flat response, keeping the harmonics
```

The pre-emphasis/de-emphasis approach is preferred because it means the saturation curve naturally acts harder on LF content without needing multiband processing.

#### 4.3.3 Dynamic Response / Hysteresis (Low-Medium Perceptual Priority)

Real transformer cores exhibit hysteresis (the B-H curve), meaning saturation onset has a slight memory effect — it depends not just on the instantaneous signal level but on recent signal history. This produces:

- **Compression-like "glue"** on LF transients — the core doesn't saturate instantly, so the first cycle of a bass transient passes relatively cleanly, and saturation ramps in over a few cycles.
- **Subtle overshoot "smear"** on release — the core doesn't unsaturate instantly either.

**Implementation approach:** A short envelope follower modulating the waveshaper drive:

```
// Simplified hysteresis via envelope-modulated drive
envelope = smooth(abs(x), attack=0.5ms, release=5ms)
dynamic_drive = base_drive + (envelope * hysteresis_amount)
```

This is lower priority than harmonic generation and frequency shaping. If CPU budget is tight, a memoryless waveshaper with the frequency-dependent drive from §4.3.2 captures 80%+ of the perceptual effect. Hysteresis can be added in a later iteration.

### 4.4 Mode-Aware Character (Internal, Not User-Facing)

Different hardware compressor topologies used different transformers. The Iron DSP subtly adjusts its internal character based on the active compressor mode, without exposing this to the user:

| Mode | Transformer inspiration | Internal adjustment |
|------|------------------------|-------------------|
| Opto | Teletronix LA-2A era — UTC/Freed input/output transformers | Slightly more HF rolloff, warmer 2nd harmonic emphasis |
| FET | UREI 1176 era — Reichenbach/UTC output transformer | Tighter LF, slightly higher 3rd harmonic content |
| PWM | Pye broadcast era — Pye/Partridge transformers | Maximum LF thickening, most pronounced HF rolloff above 10 kHz (per Waves Kramer PIE documentation) |

These are subtle shifts in 2–3 biquad/waveshaper coefficients (e.g. LF shelf gain, HF shelf frequency, waveshaper asymmetry). Coefficients update once at mode switch time, not per sample — so the per-sample cost is identical whether Iron uses one fixed character or three. The only real cost is development/tuning: tuning three character curves takes roughly three times the listening time. **Include mode-aware Iron at launch (Phase 2).**

### 4.5 Output Module Layout Update

The Output Module currently contains:

```
[ In meter ] [ Output knob ] [ Out meter ]
                [ GR readout ]
```

With Iron and Auto Gain added, the layout becomes:

```
[ In meter ] [ Iron knob ] [ Output knob ] [ Out meter ]
                   [ Auto Gain toggle ]
                   [ GR readout ]
```

- **Iron knob:** 48px diameter (slightly smaller than the 56px Output knob to establish visual hierarchy — Output is the primary control).
- **Iron accent:** `ombicYellow` (#e5a800).
- **Iron label:** "IRON" — 9px, weight 700, uppercase, pluginMuted.
- **Iron value:** Display as percentage, 14px, weight 900.
- **Position:** Between the In meter and the Output knob. The meter → Iron → Output → meter layout mirrors the signal flow (input level → transformer → gain → output level).

Meter gap adjustments may be needed to accommodate the additional knob. Target layout width should be evaluated in prototype.

### 4.6 Auto Gain (Parameter-Based Auto-Makeup)

To address "I switched modes and now it's way too loud/quiet" without fighting the design intent of different dynamics per mode, the Output module includes an **Auto Gain** toggle (parameter-based auto-makeup, similar to FabFilter Pro-C 2).

**Behavior:**

- When **Auto Gain** is on, a simple formula estimates makeup gain from the current **Threshold**, **Ratio**, and (where applicable) **Speed/Attack** settings. The formula does not measure the signal — it calculates "you're probably losing about X dB, so here's X dB back."
- Gets the user within a couple of dB for fair A/B between modes. It will not perfectly match across modes because the modes intentionally behave differently; that difference is the point.
- The formula is **not** topology-aware: use the nominal threshold and ratio values to estimate expected gain reduction. Each mode produces different *actual* GR at the same settings, but auto-makeup is close enough to audition fairly.
- Implementation: apply the estimated makeup in the same place as the Output gain stage (after Iron). Auto Gain and the Output knob can interact (e.g. Auto Gain adds +4 dB, user adds +2 dB manual = +6 dB total), or Auto Gain can override manual when on — design choice at implementation time.

**What we are not doing at launch:**

- **Measurement-based TRIM** (e.g. Tokyo Dawn–style): measure average gain reduction over a few seconds, then one-click trim to level-match. More accurate for A/B but requires playback first and extra UI (TRIM button, measurement indicator) and DSP (loudness analysis). Defer to a future release.

**Rationale:** No existing plugins do automatic per-topology normalization; the different dynamics behavior *is* the point of having multiple modes. Parameter-based auto-makeup solves the 80% case without fighting that intent.

---

## 5. Parameter Summary

### 5.1 New Parameters (PWM Mode)

These parameters are active only when Compressor Mode = PWM:

| Parameter ID | Display Name | Min | Max | Step | Default | Unit | Skew |
|-------------|-------------|-----|-----|------|---------|------|------|
| `pwmThreshold` | Threshold | 0 | 100 | 0.1 | 50.0 | % | 1.0 (linear) |
| `pwmRatio` | Ratio | 1.5 | 8.0 | 0.1 | 3.0 | :1 | 1.0 (linear) |
| `pwmSpeed` | Speed | 0 | 100 | 1 | 40 | % | 1.0 (linear) |

**Note on Threshold (resolved):** PWM reuses the same shared `threshold` parameter as Opto and FET. One knob, 0–100% range; each mode maps it internally (Opto as percentage, FET to -60…0 dB, PWM to feedback-topology reference). When the user switches modes, the knob stays where it is — presets and mode-switching remain seamless. Only revisit with a separate PWM threshold if listening tests show the same knob position feels wildly different in PWM vs. Opto/FET due to the feedback topology.

### 5.2 New Parameters (Shared — All Modes)

| Parameter ID | Display Name | Min | Max | Step | Default | Unit | Skew |
|-------------|-------------|-----|-----|------|---------|------|------|
| `iron` | Iron | 0 | 100 | 1 | 0 | % | 1.0 (linear) |
| `autoGain` | Auto Gain | 0 | 1 | 1 | 0 | — | Toggle (off/on) |

### 5.3 Updated Mode Enum

```cpp
enum class CompressorMode {
    Opto,
    FET,
    PWM   // NEW
};
```

---

## 6. Sidechain Interaction Matrix

How the sidechain filter interacts with all three modes:

| Condition | Opto | FET | PWM |
|-----------|------|-----|-----|
| **SC Freq = 20 Hz (bypass)** | Internal sidechain: LPF 2400 Hz + optional HF shelf (Limit mode) | Detector reads main buffer (post-Neon) | Internal sidechain: HPF 150 Hz (Pye-style) |
| **SC Freq > 20 Hz (active)** | External detector overrides internal LPF/shelf | External detector feeds level detection | External detector overrides internal HPF |
| **SC Listen = On** | Output replaced by sidechain signal | Output replaced by sidechain signal | Output replaced by sidechain signal |

The PWM internal HPF at 150 Hz is analogous to Opto's internal LPF at 2400 Hz — both are mode-specific detector conditioning that only runs when the shared sidechain is bypassed.

---

## 7. Compressor Mode Comparison

| Characteristic | Opto | FET | PWM |
|---------------|------|-----|-----|
| **User controls** | Threshold, Compress/Limit | Threshold, Ratio, Attack, Release | Threshold, Ratio, Speed |
| **Knob count** | 1 | 4 | 3 |
| **Topology** | Feedforward (Opto cell) | Feedforward | **Feedback** |
| **Knee** | Soft (opto cell nonlinearity) | Hard (explicit ratio) | **Soft (feedback-inherent)** |
| **Attack range** | Fixed (opto envelope) | 20–800 ms (user) | 0.5–80 ms (Speed-mapped) |
| **Release** | Fixed (opto envelope) | 50–1100 ms (user) | 30–800 ms (Speed-mapped, program-dependent) |
| **Gain element character** | Opto cell nonlinearity (slow, asymmetric) | FET variable resistance (adds harmonics) | **Clean multiplication (transparent)** |
| **Internal sidechain** | LPF 2400 Hz + optional HF shelf | None | **HPF 150 Hz** |
| **Transfer curve color** | ombicBlue | ombicBlue | **ombicTeal** |
| **Grid proportions** | Fr(85), Fr(160), Fr(70) | Px(max(320,w×0.43)), Fr(130), Fr(60) | Fr(75), Fr(160), Fr(75) |
| **Ideal use case** | Vocals, gentle leveling | Drums, aggressive compression | **Bus glue, mastering, transparent control** |

---

## 8. DSP Implementation Notes

### 8.1 PWM Compressor Chain

Create `pwmChain_` as a new compressor implementation. Core components:

1. **Feedback detector** — reads the output of the gain stage, not the input.
2. **Envelope follower** — attack/release times derived from Speed parameter via logarithmic mapping.
3. **Program-dependent release** — release time modulated by GR depth and duration.
4. **Gain computer** — soft-knee ratio curve (feedback topology inherently softens the knee; no explicit knee parameter needed).
5. **Gain stage** — clean linear multiplication. No waveshaping, no nonlinearity. This is the "clean PWM" identity.
6. **Internal sidechain HPF** — 2nd-order Butterworth at 150 Hz, active only when shared SC is bypassed.

### 8.2 Iron Processing Chain

Insert between compressor output and output gain:

1. **Pre-emphasis filter** — LF shelf boost (mode-dependent, +2–4 dB below 200 Hz scaled by Iron amount).
2. **Waveshaper** — Asymmetric tanh soft clip at 2x–4x oversampling. Drive scaled by Iron amount.
3. **De-emphasis filter** — LF shelf cut to restore flat response (inverse of pre-emphasis). Harmonic content generated by the waveshaper is preserved because it wasn't present in the pre-emphasis signal.
4. **HF shelf** — Gentle rolloff above 8–12 kHz, scaled by Iron amount (mode-dependent frequency).
5. **Dry/wet crossfade** — Iron amount controls the mix. At 0%: fully dry (bypass). At 100%: fully wet.

### 8.3 CPU Considerations

- **PWM compressor:** Computationally cheap. Feedback topology is one extra buffer read. Envelope follower and gain computer are simple per-sample operations. No oversampling needed for the compressor itself.
- **Iron:** The waveshaper oversampling (2x–4x) is the main CPU cost. At 2x, this is manageable. The pre/de-emphasis filters are two biquads (negligible). Total Iron cost should be comparable to the existing Neon stage.
- **Internal sidechain HPF:** One biquad, negligible cost.

### 8.4 Latency

- **PWM compressor:** Zero additional latency (sample-by-sample processing with feedback).
- **Iron oversampling:** If using linear-phase oversampling filters, there will be a small fixed latency (typically 16–32 samples at 2x). This should be reported to the host for PDC (plugin delay compensation). If minimum-phase oversampling is used instead, latency is zero but with slight phase shift at HF — acceptable for a saturation stage.

---

## 9. Preset Considerations

### Mode Switching Behavior

When the user switches compressor modes, parameters that don't exist in the new mode should be hidden but preserved internally. If the user switches FET → PWM → FET, their FET Attack/Release values should still be there.

The shared Threshold parameter maintains its knob position across all modes (each mode maps it differently internally).

### Factory Presets (Suggested Starting Points)

| Preset Name | Threshold | Ratio | Speed | Iron | Neon Mix | Use Case |
|------------|-----------|-------|-------|------|----------|----------|
| Bus Glue | 55 | 3.0 | 35 | 25 | 0% | Stereo bus, transparent leveling |
| Pye Vocal | 45 | 4.0 | 50 | 40 | 0% | Vocal chain, controlled warmth |
| Broadcast | 60 | 2.0 | 25 | 15 | 0% | Dialog/podcast, gentle and clean |
| Tape Return | 40 | 5.0 | 65 | 70 | 30% | Aggressive Pye character + Neon texture |
| Mastering | 65 | 2.0 | 20 | 10 | 0% | Mastering bus, nearly invisible |

---

## 10. UI Mockup Reference (Text Description)

### PWM Mode — Compressor Module

```
┌─────────────────────────────────────────────┐
│  COMPRESSOR                          [teal] │
├─────────────────────────────────────────────┤
│                                             │
│  ┌─────────────────────────────────────┐    │
│  │         Transfer Curve              │    │
│  │    (soft knee, teal, with glow)     │    │
│  └─────────────────────────────────────┘    │
│                                             │
│   THRESHOLD      RATIO        SPEED         │
│     (○)           (○)          (○)          │
│    50.0          3.0:1          40           │
│                                             │
└─────────────────────────────────────────────┘
```

### Output Module (Updated with Iron + Auto Gain)

```
┌───────────────────────────────┐
│  OUTPUT                       │
├───────────────────────────────┤
│                               │
│  ▐▌    (○)      (○)    ▐▌    │
│  In    IRON    OUTPUT   Out   │
│        [yel]   [pur]         │
│                               │
│  [ Auto Gain ]   GR: -3.0 dB  │
│                               │
└───────────────────────────────┘
```

---

## 11. Testing Criteria

### Functional Tests

1. **Mode switching:** PWM ↔ FET ↔ Opto preserves parameter values correctly.
2. **Sidechain override:** PWM internal HPF disengages when SC Freq > 20 Hz.
3. **SC Listen:** Correctly outputs sidechain signal in PWM mode, bypassing Iron and Output.
4. **Iron bypass:** At Iron = 0%, signal passes through unmodified (bit-identical or within -144 dB of bypass).
5. **Iron at all modes:** Iron functions identically (with subtle mode-aware character shifts) regardless of compressor mode.
6. **Auto Gain:** When on, output level moves toward level-matched (within a few dB) when switching modes or changing Threshold/Ratio; formula uses nominal parameters only (no signal measurement).
7. **Feedback stability:** PWM feedback topology does not oscillate or produce runaway gain reduction under any parameter combination.
8. **Program-dependent release:** Release time visibly/audibly extends on sustained loud material vs. transient material.

### Perceptual Tests

1. **PWM vs. FET transparency:** With matched GR amounts, PWM should sound noticeably cleaner/less colored in the gain reduction itself.
2. **Speed control feel:** The single Speed knob should feel intuitive — "faster = more aggressive grab" without needing to think about attack vs. release.
3. **Iron character:** At 30–50%, Iron should add warmth without obvious distortion. At 80–100%, it should sound dense and "heavy" without harsh aliasing.
4. **Internal sidechain:** With SC bypassed, PWM compression should noticeably preserve bass content better than FET at similar settings (due to the 150 Hz internal HPF).
5. **Bus test:** PWM at Threshold 55, Ratio 3.0, Speed 35, Iron 20 on a full mix should sound like "professional bus compression" — dynamic control without obvious squashing.

---

## 12. Implementation Priority

### Phase 1 — PWM Compressor Core
- New `pwmChain_` with feedback topology
- Speed-mapped attack/release with program-dependent release
- Internal 150 Hz HPF sidechain
- Mode enum update and mode switcher UI
- Transfer curve rendering in teal with soft knee
- Three-knob layout

### Phase 2 — Iron Transformer Stage + Output
- Waveshaper with oversampling
- Pre/de-emphasis frequency-dependent saturation
- HF rolloff shelf
- Iron knob on Output module
- Mode-aware internal coefficients (switch-time only; include at launch)
- **Auto Gain** toggle: parameter-based auto-makeup from Threshold/Ratio/Speed (or Attack); formula-based, not measurement-based

### Phase 3 — Polish / Future
- Factory presets
- Hysteresis/dynamic response on Iron (if CPU allows)
- Transfer curve "effective ratio" ghost line (optional)
- A/B testing against reference material (Waves Kramer PIE, real Pye recordings if available)
- **Future:** Measurement-based TRIM (Tokyo Dawn–style level-match after playback) — more accurate A/B, requires TRIM UI and loudness analysis

---

## 13. Research References

The following research informed this specification:

- **PWM gain equivalence:** Wong & Wong (2003), "The Frequency Spectrum of Pulse Width Modulated Signals" — double Fourier series proving baseband equivalence.
- **PWM emulation paradox:** KVR Forum thread (t=564327), GroupDIY PWM compressor threads — developer consensus on "nothing to emulate" in ideal PWM.
- **Pye 4060 character:** Postfade.co Pye broadcast console documentation, Waves Kramer PIE manual, Airwindows Pyewacket developer notes.
- **Transformer perceptual priority:** University of Huddersfield, "A Perceptual Evaluation of Audio Transformer Colouration" — ABX listening tests establishing 2nd/3rd harmonic and LF saturation as primary subjective factors.
- **Psychoacoustic limits of switching artifacts:** Oohashi et al. hypersonic effect research, Meyer & Moran (AES 2007) high-resolution audibility study — no robust evidence for conscious audibility of PWM carrier artifacts.
- **DSP modeling techniques:** SMC 2024 Pultec WDF paper, DAFx 2024 neural virtual analog paper, Semantic Scholar virtual analog distortion modeling survey.
- **Crane Song technical references:** STC-8 manual, Titan Squared technical overview, TransAudio PWM technical note.

Full Perplexity research threads are archived in the project Cursor folder.
