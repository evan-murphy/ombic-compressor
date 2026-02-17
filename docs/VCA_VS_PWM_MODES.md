# VCA vs PWM mode — how they work differently

**Summary:** PWM is an **algorithmic** mode (no curve data); VCA would be a **measured-curve** mode (like FET/Opto), using analyzer output from a reference plugin (e.g. dBComp/DBX 160).

---

## 1. Data and topology

| Aspect | PWM (current) | VCA (proposed) |
|--------|----------------|-----------------|
| **Curve data** | None. No CSV, no bundle dependency. | Same as FET/Opto: `compression_curve.csv` (+ optional FR/THD) from analyzer run on reference (e.g. dBComp). |
| **Gain reduction** | Formula: threshold, ratio, soft knee (±3 dB), feedback detector. | Interpolation from measured (threshold, ratio, input_db) → gain_reduction_db (MeasuredCompressor). |
| **Detector** | **Feedback** — detector reads *output* of gain stage. Optional internal 150 Hz HPF when no external sidechain. | **Feedforward** — detector from main buffer or external sidechain (same as FET). No special internal HPF. |
| **Attack/Release** | **Speed** (0–100) → single control mapped to attack/release + program-dependent release in code. No A/R knobs. | Hardware reference (dBComp) has **program-dependent** timing; no A/R in curve. UI can reuse FET knobs (Threshold, Ratio) and hide or default Attack/Release. |

So: **PWM = self-contained DSP, no reference hardware. VCA = “emulate this box” via curves, same pipeline as FET/Opto.**

---

## 2. Design philosophy

**PWM** (from `PWM_MODULE_TECHNICAL_SPEC.md` and `PWM_IMPLEMENTATION_ARCHITECTURE.md`):

- “Ideal PWM gain reduction is mathematically equivalent to sample-by-sample multiplication.”
- Sonic identity comes from: **(a)** feedback detector topology, **(b)** transformer/Neon, **(c)** output stage — **not** from measuring a specific hardware unit.
- No curve data ⇒ PWM works even when FET/Opto curve data is missing (e.g. dev builds).

**VCA** (from Perplexity research + dBComp capture):

- Emulate a **specific reference** (e.g. DBX 160 / dBComp): capture its transfer and (optionally) FR/THD, then replay via MeasuredCompressor (+ optional FR/THD character).
- “VCA character” = transparent, precise, bus-style; the **curve** defines that character, not a closed-form formula.

---

## 3. Implementation shape

| Area | PWM | VCA |
|------|-----|-----|
| **Chain class** | `PwmChain` — holds `PwmCompressor` + Neon. No `DataLoader`, no `MeasuredCompressor`. | `MVPChain(Mode::VCA, ..., vcaDataDir)` — same as FET/Opto: `loadAnalyzerOutput(vcaDir)` → `MeasuredCompressor`. |
| **Processor** | `ensurePwmChain()` builds chain with no file I/O. `hasCurveDataLoaded()` unchanged (PWM doesn’t need it). | `ensureVcaChain()` needs `dbcomp_vca` dir (bundle or dev path). Curve load same as FET/Opto. |
| **Parameters** | Threshold, Ratio (1.5–8), **Speed**. No Attack/Release. | Threshold (0–100 → map to dBComp -1..3), Ratio (1–20). Attack/Release optional (unused or defaults). |
| **UI** | Three knobs: Threshold, Ratio, Speed. PWM pill (teal). | Same knobs as FET (Threshold, Ratio; A/R hidden or defaulted). VCA pill; distinct color if desired. |
| **Transfer curve** | Teal, **soft knee** (drawn in code). | Same style as FET: curve from current threshold/ratio (from curve data / effective T&R). |

---

## 4. When to use which

- **PWM:** You want a **clean, feedback-style** bus compressor with one “Speed” control and no dependency on measured data. No reference hardware; behavior is defined by the algorithm and detector topology.
- **VCA:** You want to **recreate a specific unit** (e.g. DBX 160–style) using captured curves. Behavior is defined by the analyzer run on that unit; same “curve OK” and bundle story as FET/Opto.

So: **PWM = algorithmic, no curves, feedback, Speed. VCA = curve-based, feedforward, same data pipeline as FET/Opto, optional timing (program-dependent or default).**
