# Curve Data and Optional Regeneration

Curve data is **required** and is **part of this repo**: `output/fetish_v2` and `output/lala_v2`. The build **always** packages these into the VST3. **VCA** curve data (`output/dbcomp_vca`) is **optional**: if present, it is packaged too and the VCA chain loads; if absent, the build still succeeds and only FET and Opto modes use curve data. The compressor has **no dependency** on Ombic VST Inspector or any other tool — clone this repo and build; the data is here. For the overall architecture (two systems, roles, why this design), see **docs/ARCHITECTURE.md**.

This doc describes the data layout and, optionally, how to **regenerate or update** that data (e.g. using Ombic VST Inspector) if you have reference plugins and want to refresh the curves.

---

## Data layout (required and optional in this repo)

| Path in this repo | Purpose |
|-------------------|--------|
| `output/fetish_v2/` | FET-mode curve data: `compression_curve.csv`, `timing.csv`, `frequency_response.csv`, `thd_vs_level.json`. **Required.** |
| `output/lala_v2/` | Opto-mode curve data: same file names where applicable. **Required.** |
| `output/dbcomp_vca/` | VCA-mode curve data (DBX 160–style): same file set. **Optional** — packaged into VST3 only if `compression_curve.csv` exists. |

The build copies fetish_v2 and lala_v2 (and dbcomp_vca when present) into the VST3’s `Contents/Resources/CurveData/` every time. Fetish_v2 and lala_v2 must exist at configure time or the build fails. If `output/dbcomp_vca/compression_curve.csv` exists, the build also copies dbcomp_vca so the VCA chain can load.

---

## Optional: regenerating curve data (e.g. with Ombic VST Inspector)

If you want to **update** the curves (e.g. after re-measuring reference plugins):

1. In **Ombic VST Inspector** (separate repo): run the analyzer/ripper; it writes `output/fetish_v2/`, `output/lala_v2/`, and optionally VCA data. For VCA, use REAPER capture + `analyze-captured-batch` and then convert/export (see Inspector’s `docs/REFERENCE_DATA_PER_MODULE.md`).
2. **Export**: run `./scripts/export-curve-data-for-compressor.sh` to get `curve-data-for-compressor.tar`.
3. **Import into this repo**: run `./scripts/import-curve-data.sh /path/to/curve-data-for-compressor.tar` so this repo has the new `output/fetish_v2/`, `output/lala_v2/`, and if included, `output/dbcomp_vca/`.
4. **Build** the compressor; the new data is packaged into the VST3.

The compressor itself does not depend on Inspector; this is only for refreshing the data that already lives in this repo.
