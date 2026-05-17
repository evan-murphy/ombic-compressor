# PWM + Iron + Auto Gain — Implementation Architecture

**Purpose:** Resolve where features live (DSP vs UI), how they plug into the existing codebase, and which decisions must be locked in before implementation. This doc complements `PWM_MODULE_TECHNICAL_SPEC.md`.

---

## 1. Iron placement — where it lives

**Conclusion: Iron belongs in the final “output” stage, both in DSP and in the UI.**

| Aspect | Location | Rationale |
|--------|----------|-----------|
| **DSP** | `PluginProcessor::processBlock`, after the compressor and before (or combined with) makeup gain | Spec: “Iron sits *after* compression and *before* makeup gain.” Current flow is: `chain->process(buffer)` → (if !scListen) `buffer.applyGain(makeupGain)`. Insert Iron between those two steps. |
| **UI** | `OutputSection` (the module that already has the Output knob and IN/OUT meters) | Spec: “Iron knob on Output module”; layout “IN meter \| Iron knob \| Output knob \| OUT meter”. One module = one place for “everything after the compressor”: Iron, Auto Gain, Output gain, GR readout. |

So: **Iron processing** runs in the processor’s output stage; **Iron controls** live in `OutputSection`. No split across modules. The “final module that has the gain makeup” is exactly the right home for Iron and for the Auto Gain toggle.

---

## 2. Signal flow in code (processor)

Current high-level flow in `processBlock`:

```
1. Input metering (inputLevelDb)
2. Sidechain: mono sum → optional shared HPF → sidechainMonoBuffer_, sidechainStereoForListen_
3. Read params (mode, threshold, ratio, attack, release, makeupDb, neon*, optoCompressLimit)
4. Map threshold for FET (0–100 → -60…0 dB); Opto keeps 0–100
5. chain = (mode == 1) ? fetChain_ : optoChain_;  chain->process(buffer, ...)
6. If scListen: buffer = sidechainStereoForListen_
7. Else: buffer.applyGain(makeupGain); then output metering (outputLevelDb)
```

Target flow with PWM + Iron + Auto Gain:

```
1–3. [unchanged]
4. If mode == 2 (PWM): threshold stays 0–100 (PWM-specific mapping inside PWM chain); ratio, speed used. No attack/release.
5. chain = (mode == 0) ? optoChain_ : (mode == 1) ? fetChain_ : pwmChain_;  chain->process(buffer, ...)
   - For PWM: detector = (SC Freq > 20 Hz) ? sidechainMonoBuffer_ : pwmInternalDetector_ (mono sum → 150 Hz HPF).
6. If scListen: buffer = sidechainStereoForListen_  [unchanged]
7. Else:
   a. Iron: ironProcessor_.process(buffer, mode, ironAmount);  // mode for mode-aware coefficients
   b. makeupTotal = makeupDb + (autoGain ? estimateMakeupDb(mode, thresholdRaw, ratio, attack, release, speed) : 0);
   c. buffer.applyGain(std::pow(10.f, makeupTotal / 20.f));
   d. output metering
```

So:

- **Iron** is a new DSP block (e.g. `IronTransformer` or `emulation::Iron`) that runs in the processor after the compressor and before gain. It needs: buffer, current **mode** (for mode-aware coefficients), and **iron** parameter. It does not need Neon or sidechain.
- **Auto Gain** is a formula in the processor that adds dB to the gain stage; it does not need its own processing block.

---

## 3. Architecture decisions and potential conflicts

### 3.1 PWM is not curve-based — different from FET/Opto

- **Current:** `fetChain_` and `optoChain_` are `MVPChain`, which loads analyzer curve data and uses `MeasuredCompressor`. No curve data ⇒ no chain (standalone Neon only).
- **PWM:** Algorithmic (feedback detector, gain computer, clean gain stage). No CSV/curve data.

**Decision:** Implement PWM as a separate path, not as another `MVPChain`:

- **Option A (recommended):** Add a `PwmChain` (or `PwmCompressor` + wrapper) that contains no curve loading: Neon (optional) + feedback compressor + optional internal 150 Hz HPF on detector. Create `pwmChain_` in `prepareToPlay` or lazily when `mode == 2`; no dependency on `ensureChains()` curve data. In `releaseResources()`, reset `pwmChain_` like the other chains.
- **Option B:** Inline PWM in `processBlock` (no `pwmChain_`). Works but mixes “chain” and “inline” styles; Option A keeps one chain per mode and keeps `processBlock` simpler.

**Conflict to avoid:** Do not require curve data for PWM. `hasCurveDataLoaded()` can remain “FET or Opto data loaded”; PWM can work even when curve data is missing (e.g. dev without data).

### 3.2 Mode enum and parameter layout

- **Current:** `paramCompressorMode` = choice "Opto" (0) / "FET" (1). Combo selectedId 1 = Opto, 2 = FET. `mode` in code = 0 or 1.
- **Target:** Three modes: 0 = Opto, 1 = FET, 2 = PWM.

**Decisions:**

- Add "PWM" as third choice; selectedId 3 → mode index 2.
- **Shared:** `paramThreshold`, `paramRatio` (PWM uses ratio 1.5–8; may need to clamp ratio range when mode is PWM, or use a separate param — spec says shared ratio with constrained range for PWM; implementation can use same param and clamp in PWM path).
- **PWM-only:** `paramPwmSpeed` (0–100). Only visible when mode == 2.
- **Output module:** `paramIron` (0–100), `paramAutoGain` (bool).

**Conflict:** CompressorSection currently has `setModeControlsVisible(bool fetishParamsVisible)` — true = FET (ratio, attack, release), false = Opto (Compress/Limit). For PWM we need a third state: show Threshold, Ratio, **Speed**; hide Attack, Release, Compress/Limit.

**Decision:** Extend to three-way visibility, e.g. `setModeControlsVisible(int mode)` with mode 0 = Opto, 1 = FET, 2 = PWM. CompressorSection gains a **Speed** slider (and label), visible only when mode == 2. Same pattern as existing FET-only controls.

### 3.3 CompressorSection layout for three modes

- **Opto:** Threshold (big), Compress/Limit. (Current.)
- **FET:** Threshold, Ratio, Attack, Release. (Current.)
- **PWM:** Threshold, Ratio, Speed. (New.)

So for PWM we need:

- Threshold and Ratio sliders (already there; shared param).
- **Speed** slider (new), attached to `paramPwmSpeed`, 0–100, default 40.
- Hide Attack, Release, Compress/Limit when mode == 2.

Layout and min widths in the spec (e.g. PWM ~260px) should be reflected in `PluginEditorV2::resized()` grid (e.g. `compFr` and `minCompW` for three-knob PWM).

### 3.4 OutputSection layout and parameters

- **Current:** IN meter, Output knob, OUT meter; GR readout below. Single `paramMakeupGainDb` attachment.
- **Target:** IN meter, **Iron** knob, Output knob, OUT meter; **Auto Gain** toggle; GR readout.

**Decisions:**

- Add `ironSlider` (0–100, ombicYellow) and `autoGainToggle`; attach to `paramIron` and `paramAutoGain`.
- Order: meter | Iron | Output | meter, then row or area for Auto Gain + GR. Exact layout (compact vs full) can follow spec §4.5/§10; reserve space so the section doesn’t overflow on narrow widths.

### 3.5 Iron needs compressor mode (mode-aware character)

- Iron DSP uses 2–3 coefficients (e.g. LF shelf gain, HF shelf frequency, waveshaper asymmetry) that depend on the **current compressor mode** (Opto / FET / PWM). Coefficients update at mode switch, not per sample.

**Decision:** Iron’s `process(buffer, mode, ironAmount)` (or equivalent) takes `mode` from the processor. So the processor passes `mode` (0/1/2) into the Iron block each block; no need for Iron to hold a reference to APVTS. Keeps Iron a pure DSP unit and avoids threading issues.

### 3.6 Auto Gain formula and interaction with manual makeup

- Spec: “Parameter-based auto-makeup”; “apply the estimated makeup”; “Auto Gain and the Output knob can interact … or Auto Gain can override manual when on — design choice at implementation time.”

**Decision to lock in:** **Additive** is the simplest and matches “here’s X dB back”:  
`makeupTotal = manualMakeupDb + (autoGain ? estimateMakeupDb(...) : 0)`.  
So when Auto Gain is on, the user still has the Output knob for fine trim. If you later want “override” (Auto Gain replaces manual), that can be a second behavior behind the same toggle or a separate design pass.

**Formula:** Implement `estimateMakeupDb(mode, thresholdRaw, ratio, attack, release, speed)` using nominal threshold/ratio (and where relevant speed or attack/release) to estimate typical gain reduction in dB. Not topology-aware; one formula for all modes. Tune so that switching modes with Auto Gain on gets “within a couple of dB” for level.

### 3.7 Sidechain and PWM internal detector

- When **SC Freq = 20 Hz** (bypass): PWM uses its **internal** detector with 150 Hz HPF (Pye-style). When **SC Freq > 20 Hz**: PWM uses the shared sidechain (`sidechainMonoBuffer_`) like Opto/FET.

**Decision:** In the processor, when mode == 2 and `currentScFreq <= kScFilterOffHz`, fill a PWM detector buffer (e.g. mono sum of input or of post-compressor — spec says feedback so detector = post–gain reduction; see below) and run a **separate** 150 Hz HPF (2nd-order Butterworth). Pass that buffer to `pwmChain_->process(..., detectorBuffer)`. When mode == 2 and SC is active, pass `sidechainMonoBuffer_`. Use a dedicated `juce::dsp::IIR::Filter` (or same style as shared SC HPF) and a small buffer; no need to reuse the shared SC filter, which has a user-set frequency.

**Feedback detail:** Spec says PWM detector reads **output** of the gain stage. So the PWM chain must be implemented so that its detector is fed from the compressor output (the buffer it’s modifying), not the raw input. That’s inside PwmChain/PwmCompressor: feedback loop per block or per sample. The “internal 150 Hz HPF” is applied to that detector signal when no external sidechain is used.

### 3.8 Transfer curve (MainViewAsTubeComponent / TransferCurveComponent)

- **Current:** TransferCurveComponent uses `paramThreshold`, `paramRatio`, `paramCompressorMode`; mode 0 = Opto (softer display ratio), mode 1 = FET (hard knee, ombicBlue).
- **PWM:** Spec wants **teal** curve and **soft knee** (~±3 dB around threshold).

**Decision:** In `TransferCurveComponent::paint()`, branch on mode: when mode == 2, use `OmbicLookAndFeel::ombicTeal()` for the curve and draw a soft-knee transfer (e.g. smooth transition over ±3 dB) instead of a hard knee. Optional: “effective ratio” ghost curve for PWM; can be Phase 3.

### 3.9 Mode selector UI (pills)

- **Current:** Two pills (Opto, FET); `optoPill_`, `fetPill_`; onClick sets mode combo 1 or 2.
- **Target:** Three pills: Opto, FET, PWM (spec: PWM accent ombicTeal).

**Decision:** Add `pwmPill_`; when clicked set combo selectedId 3. In timerCallback, set toggle state for all three from combo. Teal styling for PWM pill when active.

### 3.10 Presets and new parameters

- New parameters: `paramPwmSpeed`, `paramIron`, `paramAutoGain`. Old presets won’t have them.

**Decision:** Add all with sensible defaults (e.g. Speed 40, Iron 0, Auto Gain false). JUCE APVTS will use defaults for missing IDs when loading old presets. No migration code required unless you need to enforce a specific legacy behavior.

### 3.11 Latency (Iron oversampling)

- Spec: Iron waveshaper at 2x–4x oversampling; linear-phase filters ⇒ small fixed latency; report to host for PDC.

**Decision:** If Iron uses linear-phase oversampling, call `setLatencySamples(...)` in the processor based on the oversampling factor and filter length. If you use minimum-phase, latency can be 0 and you don’t need to change PDC. Decide when implementing the oversampler.

### 3.12 Output section “IN” meter meaning

- **Current:** Output section “IN” meter = plugin **input** level (same as elsewhere). “OUT” = final output.
- **Spec:** “The meter → Iron → Output → meter layout mirrors the signal flow (input level → transformer → gain → output level).”

**Decision:** Keep current meaning: IN = plugin input, OUT = plugin output. The “mirrors signal flow” refers to the **control** order (Iron then Output), not to redefining the first meter as “post-compressor.” No extra buffer or level needed. If you ever want a “post-compressor, pre-Iron” meter, that would be a separate addition.

---

## 4. Suggested implementation order

1. **Parameters and mode enum**  
   Add param: `paramPwmSpeed`, `paramIron`, `paramAutoGain`. Extend compressor mode to three choices. No UI yet.

2. **PWM compressor (DSP)**  
   Implement `PwmCompressor` (feedback detector, speed→attack/release, program-dependent release, internal 150 Hz HPF when SC bypassed). Optionally wrap in `PwmChain` with Neon before it. In `processBlock`, branch mode 2 to `pwmChain_`; create `pwmChain_` in `prepareToPlay` or when first mode==2.

3. **Processor output stage**  
   Insert Iron: add `IronTransformer` (or similar), call it after compressor and before gain; pass mode and iron amount. Then add Auto Gain: `estimateMakeupDb(...)` and add to makeup when `paramAutoGain` is true.

4. **CompressorSection (PWM UI)**  
   Add Speed slider; extend `setModeControlsVisible` to three-way; show/hide Speed vs Attack/Release vs Compress/Limit by mode. Add PWM pill and wire mode 2.

5. **OutputSection (Iron + Auto Gain UI)**  
   Add Iron knob and Auto Gain toggle; layout IN | Iron | Output | OUT and Auto Gain + GR; attach to `paramIron`, `paramAutoGain`.

6. **Transfer curve**  
   Mode 2: teal color and soft knee in TransferCurveComponent.

7. **Polish**  
   Header/footer (e.g. “Curve OK” when curve data loaded; PWM doesn’t depend on it). Presets. Optional: Iron latency reporting if using linear-phase oversampling.

---

## 5. Files to add or touch (summary)

| Area | Add | Modify |
|------|-----|--------|
| **Processor** | — | `PluginProcessor.h/.cpp`: mode 2 branch, pwmChain_, Iron block, Auto Gain formula, param IDs, createParameterLayout, releaseResources, prepareToPlay |
| **PWM DSP** | `Emulation/PwmCompressor.cpp/.h` (and optionally `PwmChain`) | — |
| **Iron DSP** | `Emulation/IronTransformer.cpp/.h` (or equivalent) | — |
| **Compressor UI** | — | `CompressorSection`: Speed slider, setModeControlsVisible(2), resized for three-knob row |
| **Editor** | — | `PluginEditorV2`: pwmPill_, attachments for pwmSpeed, iron, autoGain; updateModeVisibility for mode 2; grid widths for PWM |
| **Output UI** | — | `OutputSection`: Iron slider, Auto Gain toggle, layout, attachments |
| **Transfer curve** | — | `TransferCurveComponent`: mode 2 → teal + soft knee |
| **Build** | — | `CMakeLists.txt`: add new source files for PwmCompressor, Iron |

---

## 6. Summary

- **Iron:** Implement in the **processor** as a block after the compressor and before gain; put the **controls** in **OutputSection**. That matches the spec and your intent to keep “final module with gain makeup” as the single place for output-stage tone and level.
- **Main architectural decisions:** PWM as its own chain (no curve data); three-way mode visibility and Speed slider in CompressorSection; Iron receives mode for coefficient choice; Auto Gain additive to manual makeup; keep IN/OUT meter meanings; optional latency reporting for Iron oversampling.
- Resolving these before coding avoids conflicting interpretations and keeps the integration consistent with the existing FET/Opto and v2 UI layout.
