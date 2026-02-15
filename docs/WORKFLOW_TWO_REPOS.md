# Curve Data and Optional Regeneration

Curve data is **required** and is **part of this repo**: `output/fetish_v2` and `output/lala_v2`. The build **always** packages it into the VST3. The compressor has **no dependency** on Ombic VST Inspector or any other tool — clone this repo and build; the data is here. For the overall architecture (two systems, roles, why this design), see **docs/ARCHITECTURE.md**.

This doc describes the data layout and, optionally, how to **regenerate or update** that data (e.g. using Ombic VST Inspector) if you have reference plugins and want to refresh the curves.

---

## Data layout (required in this repo)

| Path in this repo | Purpose |
|-------------------|--------|
| `output/fetish_v2/` | FET-mode curve data: `compression_curve.csv`, `timing.csv`, `frequency_response.csv`, `thd_vs_level.json`. |
| `output/lala_v2/` | Opto-mode curve data: same file names where applicable. |

The build copies these into the VST3’s `Contents/Resources/CurveData/` every time. They must exist at configure time or the build fails.

---

## Optional: regenerating curve data (e.g. with Ombic VST Inspector)

If you want to **update** the curves (e.g. after re-measuring reference plugins):

1. In **Ombic VST Inspector** (separate repo): run the analyzer/ripper; it writes `output/fetish_v2/`, `output/lala_v2/`, etc.
2. **Export**: run `./scripts/export-curve-data-for-compressor.sh` to get `curve-data-for-compressor.tar`.
3. **Import into this repo**: run `./scripts/import-curve-data.sh /path/to/curve-data-for-compressor.tar` so this repo has the new `output/fetish_v2/` and `output/lala_v2/`.
4. **Build** the compressor; the new data is packaged into the VST3.

The compressor itself does not depend on Inspector; this is only for refreshing the data that already lives in this repo.
