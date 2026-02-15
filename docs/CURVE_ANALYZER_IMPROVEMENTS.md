# Curve Analyzer Improvements for Better End-Compressor Reproduction

This doc lists **improvements to the curve analyzer** (Ombic VST Inspector / vst_analyzer) that would make the produced curve data better for reproduction in the end compressor plugin. It complements **CURVE_DATA_AND_IMPACT.md** (how the plugin uses the data) and **CURVE_GRID_SAMPLING_THEORY.md** (grid density and N=20).

**Canonical spec:** All of the following are now part of the **Standard** in **ombic-vst-inspector** repo: **`sampling_grid_spec.md`**. That spec is the source of truth for grid densities, timing rules, manifest, repeatability, and schema. The analyzer (grid_config.py, analyzer_controller.py, timing.py, validation.py) has been updated to implement this Standard.

---

## 1. Grid density (already specified)

- **Threshold and ratio:** Increase from 3×3 to **20×20** (or at least 5–7 per dimension) so the plugin can interpolate smoothly and avoid steppy ratio. See **CURVE_GRID_SAMPLING_THEORY.md** for the formula and cost.
- **Input level:** 25 points is fine; optionally 40 for “deep” mode.
- **Timing:** 5×5 is minimal; **20×20** if envelope feel is critical and you want interpolation in the plugin; otherwise at least **7×7** with **populated** attack_time_ms and release_time_ms.

---

## 2. Timing: fix and complete attack/release measurement

**Current issue:** In the repo’s `timing.csv`, **release_time_ms** is empty for all rows and **attack_time_ms** is only 0 or 10.667 ms. The plugin then falls back to param values or defaults, so envelope behaviour is not driven by real hardware timing.

**Analyzer improvements:**

- **Ensure GR is triggered:** Use a step level **above** the threshold used for timing (e.g. 0 dB or +6 dB) so steady-state GR is clearly &gt; 0 and the 63.2% / 36.8% targets are reachable. If threshold is -18 dB, 0 dB step is fine; document the step level in the schema.
- **Release tail length:** For slow release (e.g. 1.1 s), the time to 36.8% of steady-state can be ~500 ms. Use **SILENCE_AFTER_SEC** long enough that the envelope crosses 36.8% within the measurement window (e.g. ≥ 3 s for release up to 1.5 s). Optionally make it **adaptive** from the plugin’s max release.
- **Time resolution:** Envelope is currently sampled in **blocks** (e.g. 512 samples = 10.67 ms at 48 kHz). Very fast attack (e.g. 20 µs) will be “seen” in the first block, so you only get “0 or one block”. For better attack resolution, either:
  - Use **smaller blocks** (e.g. 64 or 128 samples) for the timing stage only, or
  - Estimate attack/release by **fitting** an exponential to the envelope and reporting τ (then attack_time_ms = τ_attack, release_time_ms = τ_release for 63.2% / 36.8%).
- **Always write both columns:** Never leave attack_time_ms or release_time_ms empty when the role exists; if measurement fails, write a **fallback** (e.g. from param range or a default) and optionally a `measurement_ok: false` flag so the plugin can choose to trust or override.

**Outcome:** Plugin gets a full timing table it can interpolate, so attack/release knobs map to real hardware-like envelope times.

---

## 3. Compression: align level definition with the plugin

The plugin derives **input_db** from **block-wise RMS** (e.g. 512 samples). The analyzer uses **full-buffer RMS** over a long steady tone.

**Alignment:**

- For **steady sine**, block RMS and long RMS match, so current setup is fine.
- **Document** in the curve schema or a small README: “input_db = RMS in dB of the test tone over the measured segment; plugin uses same definition per 512-sample block.”
- If you ever use **noise or program material** for compression curves, define level the same way (e.g. RMS over N samples) and document N so the plugin can match (e.g. same block size).
- **Optional:** Add a `block_size_used` or `level_definition` field in metadata so the plugin can validate alignment.

**Outcome:** No systematic level offset between “what was measured” and “what the plugin sees,” so the transfer curve lookup is correct.

---

## 4. Frequency response: phase and level resolution

**Current:** 30 freqs × 3 drive levels; magnitude only. Plugin does not yet use FR (characterFr = false).

**Improvements for when FR is enabled:**

- **Phase:** If the plugin will use **linear-phase FIR** from magnitude, phase is not required. If you ever do **minimum-phase** or phase-matched character, the analyzer should output **phase_deg** (or unwrapped phase) per frequency; the schema in `sampling_grid_spec.md` already allows it.
- **More drive levels:** 3 is enough for a first version; 5–7 levels give smoother level-dependent EQ if the plugin implements it.
- **Frequency spacing:** 30 points log-spaced is fine; ensure the range (e.g. 20 Hz–20 kHz) matches the plugin’s Nyquist and any internal filters.

**Outcome:** When you turn on FR character, the data supports both magnitude-only and (if needed later) phase-matched reproduction.

---

## 5. THD: resolution and optional harmonic phase

**Current:** 7 levels; THD% + H2–H10 levels. Plugin does not yet use THD (characterThd = false).

**Improvements for when THD is enabled:**

- **More levels:** 7 is workable; **11–13** (e.g. -40 to 0 in 3–4 dB steps) gives smoother level-dependent saturation and better interpolation in the plugin.
- **Harmonic phase:** Current JSON has magnitude (dB) per harmonic. For advanced waveshaping that matches waveform shape, **phase** of H2–H10 would be needed; optional for a later iteration.
- **Reference level:** Plugin’s THDCharacter uses a single “reference level” (-4 dB) to set drive. Document which level(s) the plugin should use (e.g. “drive from THD at -4 dB”) so it matches the analyzer’s level axis.

**Outcome:** When you turn on THD character, level-dependent saturation matches the measured profile more closely.

---

## 6. Schema and contract: units and param space

**So the plugin and analyzer stay in sync:**

- **Param ranges:** Document in the analyzer output (or in a small `manifest.json` per mode) the **param ranges** used for the grid (e.g. threshold -24 to 0 dB, ratio 4–12, attack 20–800, release 50–1100). The plugin already maps UI 0–100% to these; same ranges in the analyzer avoid extrapolation.
- **Units:** All timings in **ms**; levels in **dB** (dBFS or consistent reference). The plugin expects attack_param/release_param in plugin units and timing.csv in ms — the analyzer should write timing in ms and the plugin config (or docs) should state the param→ms mapping if needed.
- **Optional metadata:** `sample_rate`, `plugin_version`, `grid_version` (e.g. "standard_20x20") so you can detect mismatched or stale curve data at load time.

**Outcome:** No ambiguity on units or ranges; plugin can validate and interpolate correctly.

---

## 7. Repeatability and measurement quality

**To justify grid density and catch regressions:**

- **Repeatability runs:** For one or two (threshold, ratio) points, run the compression curve **twice** (or three times) and compute variance or max difference in gain_reduction_db. Report in a small `validation.json` or in the summary report. That caps the “useful” grid density (beyond which you’re interpolating noise).
- **Timing sanity checks:** If attack_time_ms or release_time_ms is missing for many rows, the analyzer could **warn** or **fail** the run so timing is fixed before export.
- **THD sanity:** Compare THD% at one level across runs; if variance is high, increase tone length or FFT stability.

**Outcome:** You know how stable the measurements are and whether 20×20 is within the noise floor or not.

---

## 8. Optional: compression at multiple attack/release

**Current:** Compression curve is measured at **one** attack/release setting per plugin (e.g. 0.5 ms, 400 ms for FET). The plugin uses that for the **static** transfer and gets envelope from timing.csv separately.

**Advanced:** If the **static** transfer (steady-state input→output) of the reference actually **varies** with attack/release (e.g. overshoot or crest-factor sensitivity), you could sample compression at several (attack, release) points and add those dimensions to the curve. That would increase rows (e.g. 20×20×5×5×25) and the plugin would need 4D interpolation. Only worth it if you have evidence that static transfer depends on time constants.

**Recommendation:** Keep current design (one static transfer + timing for envelope) unless you measure a clear dependence.

---

## 9. Knee (if the reference has it)

If the reference plugin exposes a **knee** (soft/hard) parameter:

- Add **knee** to the compression grid (e.g. 2–3 values) and to the CSV columns.
- Plugin can then interpolate or select by knee as well. Currently the schema supports it; the analyzer only needs to sweep it when the plugin has the role.

---

## 10. Summary table

| Area | Current | Improvement | Benefit |
|------|---------|-------------|---------|
| **Grid density** | 3×3 T×R | **20×20** (or 5–7 min) | Smooth interpolation; no steppy ratio (see CURVE_GRID_SAMPLING_THEORY.md). |
| **Timing** | 5×5; release empty, attack 0/10.67 ms | Trigger GR; longer tail; finer blocks or fit τ; always fill both columns | Plugin envelope matches hardware. |
| **Compression level** | Long-tone RMS | Document = block RMS for steady tone; same block size if using noise | No level mismatch vs plugin. |
| **FR** | 30 freqs, 3 drive, magnitude | Optional phase; 5–7 drive levels when FR character is on | Better tone match when enabled. |
| **THD** | 7 levels | 11–13 levels; document reference level | Smoother saturation vs level when enabled. |
| **Schema** | Implicit | Document param ranges, units; optional manifest.json | Plugin and analyzer stay in sync. |
| **Repeatability** | — | Run a few points twice; report variance | Justifies N; catches drift. |
| **Knee** | Not swept | Sweep if plugin has knee | Full param space. |

Implementing **grid density (20×20)** and **timing (fix + fill)** will have the largest impact on how well the end compressor reproduces the reference; the rest strengthens character (FR/THD) and long-term maintainability.
