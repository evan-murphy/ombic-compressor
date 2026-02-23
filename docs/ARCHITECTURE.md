# Architecture: Two Systems, One Product

This document explains what the curve data is, the two systems (Ombic Compressor and Ombic VST Inspector), and how they work together. It is the same in both repos so either codebase has the full picture.

---

## 1. What the curve data is

The **Ombic Compressor** plugin is a measurement-based emulation: it aims to reproduce the behaviour of reference compressors (FET-style and Opto-style), not just offer a generic compressor. To do that, the plugin needs **measured** answers to: for a given input level, what is the output? How do attack and release behave? What is the frequency response and THD vs level?

That measurement set is the **curve data**. It is stored as CSV and JSON files in a fixed layout:

- **`fetish_v2/`** — FET character: `compression_curve.csv`, `timing.csv`, `frequency_response.csv`, `thd_vs_level.json`
- **`lala_v2/`** — Opto character: same layout where applicable
- **`dbcomp_vca/`** — VCA character (DBX 160–style): same file set. **Optional** — if present, the VCA chain loads; build packages it into the bundle.

The plugin **requires** fetish_v2 and lala_v2 to run correctly. VCA mode is available when dbcomp_vca is present. It is the calibration that defines the emulation’s range and character. Without it, the product does not function as intended. Curve data is therefore **part of the product**, not an optional add-on.

**When curve data is absent:** The plugin fails visibly rather than silently. The processor does a **true bypass** (audio passes through unchanged; no compression, Neon, Iron, or makeup). The editor shows "No curve data" in the header and **disables** the compressor section, saturator section, output section, and mode pills so the user cannot interact with controls that would have no effect. This avoids the confusing case where knobs move but sound does not change.

---

## 2. The two systems

| System | Repo | Role |
|--------|------|------|
| **Ombic Compressor** | ombic-compressor | The **product**: the VST3 plugin that end users install. It **consumes** curve data at runtime (from the bundle or from an override path). The repo **contains** the curve data (`output/fetish_v2`, `output/lala_v2`; optionally `output/dbcomp_vca` for VCA mode) and the build packages it into the plugin (dbcomp_vca only when present). Standalone — no dependency on any other repo or tool. |
| **Ombic VST Inspector** | ombic-vst-inspector | The **measurement tool**: black-box analyzer that **produces** curve data by measuring reference VST/AU plugins (e.g. FETish, LALA). You run it when you want to generate or refresh the data. It is not part of the shipped product; end users never run it. |

So: **Inspector** = lab that produces the data. **Compressor** = product that ships with that data and uses it. The Compressor repo is self-contained; the Inspector repo is a separate tool used to create or update the data that the Compressor repo then owns and ships.

---

## 3. How they work together

- **At ship time:** The Compressor repo already has curve data in `output/fetish_v2` and `output/lala_v2` (committed or imported), and optionally `output/dbcomp_vca` for VCA mode. The build bundles fetish_v2 and lala_v2 always, and dbcomp_vca when present. End users get one plugin with the emulation baked in. They do not need the Inspector or the reference plugins.

- **When updating data:** If you want to regenerate or refresh the curve data (e.g. after re-measuring reference plugins), you run **Inspector** to produce new `output/fetish_v2` and `output/lala_v2`, then export a tarball. You **import** that into the **Compressor** repo (e.g. `scripts/import-curve-data.sh`), so the Compressor repo again owns the data. Then you build the Compressor as usual; the new data is packaged. See **docs/WORKFLOW_TWO_REPOS.md** in the Compressor repo (or the same workflow doc in the Inspector repo) for the export/import steps.

- **No dependency:** The Compressor does **not** depend on the Inspector to build or run. The Compressor repo contains the curve data. The Inspector is only used when you choose to generate or update that data. Two systems, one product: the product is the Compressor plugin plus its data; the Inspector is the tool that can produce or update that data.

---

## 4. Why this design

- **Single product for users:** One plugin, one install, no “run this other tool first.” Curve data is always packaged with the plugin.
- **Separation of concerns:** Measurement (Python, flexible, lab) lives in Inspector. Real-time DSP (C++, JUCE) lives in the Compressor. The plugin does not run the analyzer; it only reads precomputed curve data.
- **Clear ownership:** The Compressor repo owns the curve data it ships. The Inspector repo is the pipeline that can create or refresh that data when you decide to.

---

## 5. Where to find more

- **Compressor repo:** Build and curve data layout — **README.md**, **docs/WORKFLOW_TWO_REPOS.md**. What the curve data does in the plugin — **docs/CURVE_DATA_AND_IMPACT.md**.
- **Inspector repo:** How to run the analyzer and export data — **README.md**, **docs/WORKFLOW_TWO_REPOS.md**, **docs/BATCH_ANALYSIS.md**.
