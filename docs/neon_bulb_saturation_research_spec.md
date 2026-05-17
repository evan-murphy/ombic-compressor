# Neon Bulb Noise Generator for Tape-Like Saturation — Research & Technical Spec

**Purpose:** Academic research exercise to assess whether Michael Blackmer’s neon-bulb–modulated-VCA concept for tape-like saturation can be modeled digitally, and to produce a technical spec that aligns with this VST project. This is a **saturation/color** concept (stochastic gain modulation), distinct from the Pye compressor’s envelope-based gain and transformer saturation; it could sit in the “output stage” or “character” path of the emulation, or as a separate saturation mode.

**Status:** All four Perplexity research queries are incorporated from a single combined file in `docs/`; see §2.3 and §4.2.

---

## 1. Concept Summary (Analog Reference)

- **Circuit:** Starved neon bulb → random noise → high-impedance buffer → **gain control input of a VCA**. Audio passes through the VCA; the neon-derived signal modulates gain. Result: tape-like saturation (or more than tape) because you can drive more noise into the gain port than tape’s natural modulation.
- **Why neon (vs resistor/semiconductor noise):** Resistor (Johnson–Nyquist) and semiconductor (shot, 1/f) noise were found “not good candidates” — they are more statistically predictable. Starved neon ionization/deionization is a **stochastic process** with different spectral and amplitude distribution: more bursty, chaotic, and closer to tape’s complex modulation (flutter, oxide irregularities, amplitude instability).
- **“Random sampling rate”:** Discharge events are **irregular in time**, so gain modulation is aperiodic — avoiding the telltale sound of periodic modulation while mimicking analog tape’s natural instability.
- **Precedent:** Neon relaxation oscillators (chaotic near threshold), gas-discharge noise generators (e.g. General Radio 1390-B with 6D4 gas tube), and David Blackmer’s VCA lineage; physics: gas discharge near breakdown (Townsend avalanche) gives genuinely stochastic amplitude variations with distinct spectral character.

**Design insight:** Tape character is not only harmonic (waveshaping) but **stochastic modulation**. This circuit targets that modulation dimension via an analog noise process; digital emulation would need to approximate that **modulation process**, not the neon physics per se.

---

## 2. Digital Modeling: Feasibility Assessment

### 2.1 Is it possible? — Yes, in a “behavioral” sense

We do **not** need to simulate electrons and gas discharge in real time. We need to:

1. Produce a **modulation signal** that has similar **statistical and spectral** properties to the noise that would drive the VCA’s gain input in the analog circuit.
2. Apply that modulation as **gain** to the audio (or to a gain factor in the signal path), possibly with simple filtering and scaling to match perceived “tape-like” saturation.

So the question becomes: **Can we generate a digital signal that approximates the relevant properties of starved-neon noise (and optionally the buffer/VCA gain law)?**

### 2.2 What we know from literature (brief)

- **Gas discharge noise:** Documented in test equipment (e.g. GR 1390-B: gas tube, 5 c/s–5 Mc/s, ±1 dB 20 c/s–20 kc/s). So the idea of “gas tube as noise source” in (audio) bandwidth is established.
- **Starved neon / relaxation chaos:** Neon-bulb relaxation oscillators are known to exhibit chaotic behavior near threshold (e.g. Kennedy & Chua; period-adding route to chaos). “Starved” operation pushes the system into irregular, non-periodic discharge — consistent with “random sampling rate” and bursty amplitude.
- **Spectral character:** Gas discharge and low-current Townsend-type discharge show complex dynamics and stochastic behavior; exact audio-band **power spectral density** (PSD) for a starved neon in a Blackmer-style circuit is not written down in the quick scan. That is a **gap** that Perplexity or targeted literature could fill.
- **Tape emulation via modulation:** Plugins already use **wow & flutter** (random or quasi-random modulation of delay/pitch/gain) for tape character (e.g. Goodhertz Wow Control, MorphoIce Vintage Tape Wow & Flutter). So “stochastic gain modulation for saturation/character” is already in the plugin world; the novelty here is the **source** of the modulation (neon-like statistics) rather than the idea of modulating gain.

**Conclusion:** Digitally we can:
- Implement **stochastic gain modulation** (well within standard DSP).
- **Approximate** the statistics of the modulation (e.g. burstiness, bandwidth, amplitude distribution) using a **synthetic noise model** plus filtering and possibly simple nonlinearity, **without** simulating the neon physically.

Feasibility therefore hinges on: (a) how well we can **characterize** the neon noise (measurements or published PSD/temporal stats), and (b) how well we can **approximate** that with a compact digital model (noise type, filter, gain mapping).

### 2.3 Research findings (Perplexity: all four queries in one combined doc)

A single Perplexity document in `docs/` contains all four query answers (filename: `Documented or measured power spectral density (PSD.md`). Summaries:

- **Query 1 — Gas-discharge PSD and temporal statistics:** Stable glow regime → flat PSD, Gaussian, short correlation; GR 1390-B filters to 20 kHz. **No** published starved NE-2 audio-band PSD/event-rate. **Implication:** Use filtered white (or pink) + optional nonlinearity or parametric burst model; bandwidth sub–20 kHz (e.g. few kHz for modulation). Tune by ear.
- **Query 2 — DSP synthesis of bursty / gas-discharge–like noise:** Tape plugins use filtered white/pink + level-linked or modulated noise; explicit gas-discharge point processes are rare. Research: TAPESTREA-style transient/event layers (point process over noise bed), Poisson or Markov-modulated event triggers, relaxation-oscillator–derived modulators. **Implication:** Base colored-noise bed + point process (Poisson or Markov) triggering short pulses; optional Markov/AR envelope on event rate for bursts. Gamechanger Plasma is hardware discharge, no published DSP model.
- **Query 3 — Perceptual role of modulation vs harmonics:** Harmonics (odd-order, 3rd) dominate “warmth” and cohesion; modulation (wow/flutter) adds “liveness” but is secondary and can be corrected in restoration. Design priority: nonlinearity first, modulation as optional tweak. **Implication:** Neon-style stochastic gain modulation is an aesthetic layer on top of saturation; keep it subtle so it doesn’t dominate or fatigue.
- **Query 4 — Blackmer VCA gain-control and noise modulation:** Exponential control law (e.g. 20 dB/V), wideband capable but typically low-pass filtered (tens–hundreds of Hz) in application to avoid feedthrough. **No** documented use of deliberate noise on the control input for tape-like effect; control-port noise is minimized in patents and app notes. **Implication:** Random modulation of gain in software is a designer choice, not “authentic” Blackmer behavior. Model gain as exponential in a control parameter; add stochastic modulation in a **separate** block (e.g. low-rate AM or wow/flutter-style process) rather than as literal VCA control noise.

---

## 3. Technical Spec (Aligned with This VST Project)

### 3.1 Where it fits in the current architecture

From `pye_emulation_spec.md`, the chain includes:

- Optional **transformer emulation** (LF saturation, 2nd/3rd, HF roll-off).
- **Gain multiply** `y = g * x` (from sidechain for compression).
- **Output stage:** make-up, soft saturation, optional HF shelf.
- **Optional hum + noise.**

The neon concept is a **modulation layer** that could:

- **Option A — Saturation/character block:** Sit in the “output stage” or a dedicated “saturation” block: a **gain modulator** driven by synthetic “neon-like” noise, then optional soft clip. No compression; just stochastic gain variation + optional harmonic saturation.
- **Option B — Combined with existing color:** The same modulation could **multiply** the signal after the main gain stage (or after transformer emulation), so the “tape-like” instability is layered on top of existing 2nd/3rd and HF roll-off.
- **Option C — Standalone “tape saturation” mode:** A separate mode or sub-plugin: input → optional drive → **stochastic gain modulation** (neon-like) → optional saturation → output.

For MVP, **Option A or B** keeps one codebase (Pye-style compressor + optional neon-style saturation); **Option C** is a clear extension.

### 3.2 Block diagram (neon-style saturation block)

```
Audio in
    │
    ▼
[Optional: drive / pre-emphasis]
    │
    ▼
× g_mod[n]   ◄─── [Neon-like modulation generator]
    │                  │
    │                  ├─ Noise source (see 3.3)
    │                  ├─ Bandwidth / shaping filter
    │                  └─ Gain mapping (noise → linear g_mod)
    │
    ▼
[Optional: soft saturation / output stage]
    │
    ▼
Audio out
```

### 3.3 DSP elements (proposed)

| Element | Role | Proposed approach |
|--------|------|--------------------|
| **Noise source** | Provide irregular, bursty modulation | **Option (i):** Filtered white (or pink) + nonlinearity (e.g. soft clip, expander) to create burstiness. **Option (ii):** Parametric model (e.g. Markov, or “event” model with random inter-event times and amplitudes). **Option (iii):** Recorded or precomputed “neon-like” noise sample, looped with random start/phase (least physically motivated, but fast). |
| **Bandwidth / shaping** | Match effective bandwidth of neon + buffer | Lowpass to sub–20 kHz (e.g. modulation band to few kHz); optional highpass to avoid sub‑100 Hz rumble. GR 1390-B style: flat to 20 kHz then filter; for gain modulation, limiting to few kHz is reasonable (see §2.3). |
| **Gain mapping** | Noise → linear gain | Map modulation signal to `g_mod[n]` in [0, 1] or [g_min, 1] so that average gain is close to 1 (or a set “saturation” amount) and fluctuations are symmetric or slightly asymmetric. |
| **Rate of variation** | “Random sampling rate” | Use **sample-rate** modulation (no fixed LFO); randomness comes from the noise process. Optional: smooth with one-pole to avoid zipper and to mimic limited bandwidth of the analog path. |

### 3.4 Parameters (saturation block)

- **Amount / depth:** How much the gain is modulated (e.g. scale of `g_mod` deviation from 1).
- **Tone / bandwidth:** Cutoff(s) of the shaping filter (e.g. how much HF in the modulation).
- **Character / burstiness:** If using a parametric model, a control that moves from “smooth” (e.g. more like filtered white) to “bursty” (more neon-like).
- **Optional:** Mix dry/wet for the modulated path.

### 3.5 What the analyzer could (optionally) feed

- The **existing analyzer** (compression, THD vs level, FR) does not directly measure “stochastic gain modulation.” To tune a neon-style block you would:
  - Use **listening** and possibly **level statistics** (e.g. variance of gain over time) on hardware if you had a Blackmer-style unit; or
  - Tune by ear and by comparison to tape plugins (wow/flutter, saturation) and to any published description of the analog unit.
- If you later add **modulation depth / spectrum** measurements to the analyzer, those could feed this block (e.g. target variance of gain, or target modulation bandwidth).

---

## 4. Approximation Strategy and Open Questions

### 4.1 Strategy

1. **Characterize (as far as possible):** Get PSD and temporal statistics (e.g. burst rate, amplitude distribution) for starved-neon noise in audio band — from papers, test equipment manuals, or (if available) measurements of a real circuit.
2. **Match statistically:** Choose a **minimal** digital model (noise type + filter + optional nonlinearity) that approximates those statistics.
3. **Implement:** One noise generator + filter + gain mapping; keep CPU low (no physical gas simulation).
4. **Tune by ear:** Compare to tape saturation and wow/flutter plugins; align “amount” and “character” with project goals.

### 4.2 Open questions and status after research

| Question | Status | Notes |
|----------|--------|--------|
| **Spectral shape** (PSD of starved-neon in 20 Hz–20 kHz) | **Answered (gap in literature)** | Perplexity doc in `docs/`: stable gas-discharge ≈ flat/white PSD; starved NE-2 not published. Use filtered white (or pink) as approximation; optional nonlinearity for “bursty” character. |
| **Temporal structure** (event rate, amplitude distribution) | **Answered (gap in literature)** | Same doc: stable regime treated as Gaussian, short correlation; no published event-rate or PDF for starved NE-2. Proceed with white/pink + optional shaping. |
| **Level and bandwidth at VCA gain input** | **Answered** | Same combined doc (query 4): exponential law (e.g. 20 dB/V), control bandwidth typically tens–hundreds of Hz; no documented noise-on-control for tape effect. Use exponential gain mapping; keep stochastic modulation in a separate block. |
| **Psychoacoustics** (modulation vs harmonics in tape) | **Answered** | Same combined doc (query 3): harmonics dominate warmth/cohesion; modulation is secondary, adds “life” but keep subtle. Neon-style modulation = aesthetic layer, not primary character. |

**Perplexity corpus in `docs/`:** One **combined** file contains all four query answers: `Documented or measured power spectral density (PSD.md`. Sections are separated by `---` and `#` headings (query 1 → query 2 → query 3 → query 4).

---

## 5. Perplexity Queries (Deep Research)

**Practice:** See **`docs/perplexity_usage.md`** for what Perplexity is good at (retrieval, citations, papers, specs) and what it is not (deep reasoning, circuit analysis, novel synthesis). Run each query below as a **separate turn**; use **search-friendly, expert terminology** and **specific scope**; rely on **built-in citations** (do not ask for URLs in the answer text). Save each full response as Markdown in `docs/` or `Perplexity/<short_descriptive_name>.md` and cite in this spec and implementation notes. For multi-angle technical coverage, use Perplexity **Deep Research** mode (Pro) where available.

**Suggested queries (one per document; stage across multiple turns):**

1. **Spectral and statistical characterization of gas-discharge noise in the audio band**  
   Documented or measured power spectral density (PSD), bandwidth, and temporal statistics (amplitude distribution, autocorrelation, event rate) of noise from a starved neon bulb or gas-discharge tube operated near breakdown, in the 20 Hz–20 kHz range. Include: General Radio 1390-B or similar test equipment specs, relaxation-oscillator chaos literature (e.g. Kennedy–Chua), and any AES or IEEE references. Scope: 1960s–present.

2. **Digital synthesis of bursty or non-Gaussian noise for audio modulation**  
   Published DSP methods or plugin implementations for synthesizing bursty or gas-discharge–like stochastic noise for audio: filtered white/pink vs parametric models (Markov, point process, relaxation-oscillator–derived). Focus on tape or analog-character emulation. Include AES/DAFx or manufacturer white papers if available. Scope: 2000–2025.

3. **Tape saturation emulation: perceptual role of modulation vs harmonics**  
   Relative perceptual contribution of (a) harmonic saturation (THD, waveshaping) versus (b) random or quasi-random gain or delay modulation (wow, flutter, amplitude instability) in tape saturation and tape emulation plugins. Cite listening studies, plugin design notes, or AES papers. Scope: 2010–2025.

4. **Blackmer VCA gain-control input: typical use and noise modulation**  
   In dbx or Blackmer-style VCAs: gain control input characteristics (linear vs exponential law, bandwidth, typical level). Any published or documented use of noise or random modulation on the gain control input for saturation or tape-like effect. Scope: patents, application notes, or engineering literature.

Running (1) and (2) supports the “approximation strategy”; (3) and (4) support design rationale and parameterization. Use a reasoning model (e.g. in Cursor) to synthesize conclusions from the saved outputs — Perplexity retrieves and summarizes; it does not derive what is worth modeling in software.

---

## 6. Risks and Scope

- **No hardware or capture:** If we never have a real neon-modulated unit, the spec remains **behavioral** and we tune by ear and by analogy to tape/wow-flutter.
- **CPU:** Keeping the modulation generator lightweight (no physical simulation) keeps risk low; one noise source + biquad(s) + gain multiply is cheap.
- **Scope for MVP:** This is a **candidate saturation/character option**, not a replacement for the existing transformer + output saturation in the Pye spec. MVP can ship without it; this doc supports a later “neon-style saturation” or “tape-style modulation” block.

---

## 7. Summary

- **Concept:** Starved neon → noise → VCA gain control → tape-like saturation via **stochastic gain modulation**.
- **Digital feasibility:** **Yes.** Model the **modulation signal** (statistics and spectrum), not the neon physics. Stochastic gain modulation is standard DSP; precedent exists in wow/flutter and tape plugins.
- **Technical approach:** Noise generator (filtered white/pink or parametric bursty model) → shaping filter → gain mapping → multiply with audio; optional saturation after. Parameters: amount, bandwidth, burstiness.
- **Alignment with project:** Fits as optional block in output/character path of the existing Pye-style emulation, or as a separate “tape-style saturation” mode. Analyzer can support tuning only indirectly (listening, level stats) unless modulation-specific metrics are added later.
- **Next steps:** (1) Implement prototype: filtered white (or pink) → optional point-process/transient layer for burstiness → gain mapping (exponential if matching VCA) → multiply; bandwidth per §3.3; keep modulation subtle per §2.3. (2) Tune by ear against tape/wow-flutter plugins.

---

## 8. Implementation specification (buildable recipe for VST)

This section turns the spec into a concrete recipe a developer can implement.

### 8.1 Per-sample signal flow

1. **Generate modulation signal** (each sample or each block):
   - `noise_raw[n]` = white noise (e.g. uniform or Gaussian RNG), or pink (filtered white).
   - Optional: add **burst layer**: Poisson (or Markov) process that triggers short pulses (e.g. 1–20 ms envelope) at random times; mix with `noise_raw` for more “event-like” character.
2. **Shape bandwidth:**
   - One-pole lowpass: `noise_lp[n] = α · noise_lp[n-1] + (1−α) · noise_raw[n]`, with α from cutoff (e.g. 500 Hz–5 kHz for modulation band). Optional one-pole highpass to remove sub-100 Hz.
   - Or biquad lowpass + optional highpass for steeper slopes.
3. **Gain mapping (noise → linear gain):**
   - Normalize so mean is 0 and scale by depth: `c[n] = depth · (noise_lp[n] − μ) / σ` (μ, σ from running mean/variance or fixed).
   - Map to linear gain (symmetric around 1):  
     `g_mod[n] = 1 + c[n]`  
     or clamp: `g_mod[n] = clamp(1 + c[n], g_min, 1 + depth)` so gain never goes below `g_min` (e.g. 0.8) or above 1 + depth.
   - Optional **exponential** (VCA-style): `g_mod[n] = 10^(k · c[n])` with k small (e.g. 0.02–0.05 per unit c) so a few dB variation.
4. **Apply to audio:**
   - `y[n] = g_mod[n] · x[n]`.
5. **Optional:** One-pole smooth `g_mod` before multiply to reduce zipper: `g_smooth[n] = β · g_smooth[n-1] + (1−β) · g_mod[n]`, β for ~50–200 Hz effective bandwidth.
6. **Optional after multiply:** Soft saturation (tanh or polynomial) for extra tape-style harmonic saturation.

### 8.2 Suggested parameter ranges (MVP)

| Parameter | Suggested range | Notes |
|-----------|-----------------|--------|
| **Depth / amount** | 0 … 0.05 (linear) or 0 … 3 dB (if exponential) | Keep modulation subtle; research says modulation is secondary to harmonics. |
| **Modulation bandwidth** (lowpass) | 200 Hz – 5 kHz | Lower = slower wobble; higher = more “hissy” modulation. |
| **Burstiness** (if point process) | 0 = off, 1–10 events/sec | Optional; increases CPU slightly. |
| **g_min** | 0.85 – 1.0 | Prevents gain from dipping too low. |
| **Dry/wet** | 0 … 100% | Blend unmodulated and modulated for safety. |

### 8.3 State and CPU

- **State:** RNG seed or state; one-pole (or biquad) filter state; optional running mean/variance for normalization; optional Poisson/Markov event timer.
- **CPU:** One RNG + a few multiplies/adds per sample; one or two one-pole filters; one multiply for gain. Suitable for real-time on any modern host.

### 8.4 Output

- **Plugin API:** Stereo: apply same `g_mod[n]` to L and R (or independent noise per channel for wider image; spec leaves that as an option). Parameters: depth, bandwidth, burstiness (if implemented), dry/wet, optional saturation on/off.

---

**Document version:** 1.1  
**Date:** 2025-02-14  
**References:** Concept summary (user-provided); web search (gas discharge noise, Townsend avalanche, GR 1390-B, tape wow/flutter, relaxation oscillator chaos); Perplexity research in `docs/Documented or measured power spectral density (PSD.md` (combined file: all four queries — gas-discharge PSD/temporal stats, DSP synthesis of bursty noise, perceptual role of modulation vs harmonics, Blackmer VCA gain-control and noise); pye_emulation_spec.md; research_adequacy_assessment.md; docs/perplexity_usage.md.
