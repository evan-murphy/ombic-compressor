# How the Two Repos Work Together

Two separate codebases, one workflow: **Ombic VST Inspector** produces curve data; **Ombic Compressor** bundles that data into the plugin.

---

## Data roles

| Repo | Data role | What lives there |
|------|-----------|------------------|
| **Ombic VST Inspector** | **Raw data** | All measurement output: `output/fetish_v2/`, `output/lala_v2/`, plus any other plugins you’ve captured. Validation reports, multiple runs, re-captures. This is the *source* of curve data. |
| **Ombic Compressor** | **Finished curve data** | Only the curve data that gets **bundled into the plugin**: `output/fetish_v2/` and `output/lala_v2/` (same layout: `compression_curve.csv`, `timing.csv`, `frequency_response.csv`, `thd_vs_level.json`). The plugin build copies these into the VST3’s `Contents/Resources/CurveData/`. |

So: Inspector = “everything we measured.” Compressor = “the two datasets we ship inside the plugin.”

---

## Flow (one direction)

```
[Ombic VST Inspector]                    [Ombic Compressor]
  Run ripper/analyze on                     output/fetish_v2/
  FETish, LALA, etc.    ── copy ──►        output/lala_v2/
  → output/fetish_v2/                       ↓
  → output/lala_v2/                         CMake build bundles them
  (raw data)                                into the .vst3
```

1. In **Inspector**: run `vst-analyzer ripper` (or analyze) on your reference plugins. You get `output/fetish_v2/` and `output/lala_v2/` (and optionally other `output/<name>/`).
2. When you’re happy with a run, **copy** `output/fetish_v2` and `output/lala_v2` from Inspector into the **Compressor** repo’s `output/` (or use the export script below).
3. In **Compressor**: build the plugin. The existing CMake POST_BUILD copies `output/fetish_v2` and `output/lala_v2` into the VST3 bundle, so the shipped plugin contains that finished curve data.

---

## Making it easy

### From Inspector: export bundle-ready data

Run this from the **Ombic VST Inspector** repo when you want to update the plugin’s curves:

```bash
./scripts/export-curve-data-for-compressor.sh
```

That creates `curve-data-for-compressor.tar` (or copies to a path you choose) containing only `fetish_v2/` and `lala_v2/` with the expected files. You can then untar that into the Compressor repo’s `output/` (or use the import script in Compressor).

### From Compressor: import curve data

In the **Ombic Compressor** repo, either:

- **Manual:** Unpack the tarball (or copy the two folders) so you have `output/fetish_v2/` and `output/lala_v2/` at the repo root. Then build.
- **Script:** Run `./scripts/import-curve-data.sh /path/to/curve-data-for-compressor.tar` (script provided in Compressor repo).

After import, `output/` in Compressor contains only the finished data that gets bundled; build as usual.

---

## Summary

- **Inspector** = single place for *all* raw data (FETish, LALA, other plugins, multiple runs). Keep the codebase separate; keep the data there (or in a dedicated data repo if you prefer).
- **Compressor** = only the *finished* curve data that is packaged into the plugin. Same CSV/JSON layout; no extra runs or other plugins.
- **Together:** Generate in Inspector → export/copy fetish_v2 + lala_v2 → put them in Compressor’s `output/` → build. The plugin you ship has that curve data bundled inside it.
