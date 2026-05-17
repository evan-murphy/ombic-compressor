# Emulation Readiness Evaluation

**Question:** Do we have what we need to implement the VST3 emulation plugin (the “copy” that uses measured analyzer data)?

**Short answer: Yes.** The Standard analyzer run produces the data contract described in `sampling_grid_spec.md` and consumed by the architecture in `pye_emulation_spec.md`. We can implement a plugin that loads the CSVs/JSON and reproduces compression, optional timing, FR, and THD.

---

## 1. What We Have (After LALA + FETish Standard Runs)

| Data | LALA | FETish | Emulation use |
|------|------|--------|----------------|
| **Compression** | `compression_curve.csv` (225 rows: 3 threshold × 3 ratio iterations × 25 input levels; ratio/attack/release empty) | Same schema, 225 rows with threshold/ratio/attack_ms/release_ms | Lookup or interpolate `(threshold, ratio?, input_db)` → `output_db` or `gain_reduction_db`; apply gain = 10^(-GR/20). |
| **Timing** | Skipped (opto, no attack/release params) | `timing.csv` (25 rows: 5×5 attack×release) | Interpolate `(attack_param, release_param)` → `attack_time_ms`, `release_time_ms` for envelope follower. LALA: use fixed opto-style times or single default. |
| **Frequency response** | `frequency_response.csv` (90 rows: 30 freqs × 3 drive levels) | Same | Apply magnitude curve (and optionally phase) in “character” path; level-dependent if we have multiple drive levels. |
| **THD** | `thd_vs_level.json` (7 levels, THD% + H2–H10) | Same | Drive saturation/waveshaper so THD vs level matches measured profile. |

So we have:

- **Transfer curve:** Input level + threshold (and for FETish, ratio, attack, release) → gain reduction. Enough for a **gain element** that matches the measured static curve.
- **Envelope:** For FETish we have attack/release time constants; for LALA we don’t (opto is fixed). Emulation can use fixed times for LALA or a single “speed” default.
- **Tone:** FR and THD tables support the “character” path (EQ + saturation) in the emulation spec.

---

## 2. Gaps and Choices

- **LALA timing:** No timing CSV. Options: (1) Fixed attack/release (e.g. 10 ms / 60 ms), (2) One-off manual measurement and hardcode, (3) Leave envelope at default and only match transfer + FR + THD. MVP: fixed or default envelope.
- **Parameter mapping:** LALA uses `peak_reduction` 0–100%; our CSV stores that as `threshold` (25, 50, 75). Emulation UI can expose “peak reduction” or “threshold” and use the same lookup.
- **Interpolation:** Compression grid is discrete. We need 1D (input_db) or 2D (threshold, input_db) interpolation; for FETish, 4D (threshold, ratio, attack, release) is possible but heavier. Linear or cubic interpolation in (threshold, input_db) is enough for LALA; same approach scales to more dimensions.
- **Real-time envelope:** The spec’s envelope follower (attack/release smoothing) is standard DSP; timing CSV only tunes the time constants, not the structure.

---

## 3. Conclusion

We have what we need to write the VST3 emulation plugin:

1. **Core:** Load `compression_curve.csv`; given (threshold, input_db) [and optionally ratio, attack, release], interpolate gain reduction and apply `y = x * 10^(-GR/20)`.
2. **Envelope:** Use timing data (when present) for attack/release constants; otherwise use fixed/default times.
3. **Character (optional):** Load FR and THD; apply magnitude curve and level-dependent saturation to match measured tone.

A Python prototype that loads the LALA (or FETish) output directory and implements the transfer curve + optional FR/THD is sufficient to validate the data path; the same logic can be ported to C++/JUCE for a loadable VST3.
