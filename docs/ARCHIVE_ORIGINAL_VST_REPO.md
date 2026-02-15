# Archive: Original VST Monorepo

This document preserves context from the original **VST** monorepo (single repo that contained both the analyzer and the plugin). That repo has been **split**; this repo and **ombic-vst-inspector** are the canonical homes.

---

## What happened

- **Original layout:** One repo (“VST”) contained:
  - **vst_analyzer/** — Python black-box analyzer/ripper for VST/AU plugins
  - **Plugin/** — JUCE VST3 compressor + neon saturator (this product)
  - **plugin_configs/**, **scripts/**, **docs/**, **emulation/**, etc.

- **Split:** The repo was split into two:
  - **ombic-compressor** (this repo): Plugin only — JUCE build, GUI, emulation, curve data bundle.
  - **ombic-vst-inspector**: Analyzer only — Python `vst_analyzer`, plugin_configs, ripper, export script.

- **Data flow:** Inspector produces curve data → export script → import into this repo’s `output/` → build bundles it into the VST3. See **docs/WORKFLOW_TWO_REPOS.md**.

---

## Where to find things now

| Former location (VST) | Now lives in |
|----------------------|--------------|
| Plugin/, CMakeLists.txt, Plugin/CMakeLists.txt | **ombic-compressor** (this repo) |
| vst_analyzer/, plugin_configs/, scripts/export-curve-data-for-compressor.sh | **ombic-vst-inspector** |
| docs/GUI_SPEC.md, TESTING_PROTOCOL.md, WORKFLOW_TWO_REPOS.md, etc. | **ombic-compressor/docs/** or **ombic-vst-inspector/docs/** as appropriate |
| REPO_SPLIT.md, README_OMBIC_*.md | Superseded by this archive and each repo’s README |

---

## Deprecated / historical

- **VST repo root:** If you still have the old “VST” folder, treat it as read-only. Its README should point to **ombic-compressor** and **ombic-vst-inspector**. No active development there.
- **Split script:** `scripts/split_repos.sh` was used once to create the two repos; no need to run again unless re-splitting from a backup.

This archive exists in both repos so that either codebase retains the context of the original monorepo and the split.
