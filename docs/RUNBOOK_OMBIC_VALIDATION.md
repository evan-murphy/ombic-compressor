# Runbook: Ombic Compressor validation

Automated validation (analyzer + E2E compare) is run from the **Ombic VST Inspector** repo. This doc points you there and summarizes the steps.

---

## Where the runbook lives

**Full runbook (copy-paste commands, prerequisites, Option A/B):**  
In the **ombic-vst-inspector** repo, open **`docs/RUNBOOK_OMBIC_VALIDATION.md`**.

---

## Quick summary

| Goal | Where | What to run |
|------|--------|-------------|
| **Debug:** Does the compressor respond? | Inspector repo | `./scripts/run_analyzer_ombic.sh` → check `output/ombic_compressor/validation_report.json` |
| **E2E:** Does the compressor match FETish/LALA? | Inspector repo | Analyze reference → copy data into this repo’s `output/` → build this repo → analyze Ombic → run `compare_compressor_curves.py` (see full runbook) |
| **Refresh curve data** (e.g. new timing) | Inspector → this repo | Inspector: run analyzer (standard mode); export tarball. This repo: `./scripts/import-curve-data.sh <tarball>` then build. |

---

## Prerequisites

- **ombic-vst-inspector** cloned, venv set up (`pip install -e .`).
- **Ombic Compressor** built; VST3 at `~/Library/Audio/Plug-Ins/VST3/Ombic Compressor.vst3` (or set `OMBIC_VST3_PATH`).
- For E2E: reference plugins (e.g. FETish, LALA) installed.

See the full runbook in the Inspector repo for exact commands and Option A (debug only) vs Option B (full E2E).
