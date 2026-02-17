# Docs & research organization plan

**Goal:** One clear place for all docs; keep everything but organize so GitHub and future-you can find it. Perplexity research stays, with readable filenames.

---

## What to package with the GitHub repo

**Include everything** that documents the product, build, design, and research. The only things to exclude are build artefacts (`build/`), IDE config (e.g. `.cursor/`), and binary assets that belong in release packages. All markdown, HTML mockups, and specs should be in the repo under a clear structure.

---

## Current problems

1. **Two doc roots:** `docs/` (canonical) and `design-spec-files/` (specs + Perplexity exports with long prompt-as-filename names).
2. **Perplexity exports:** Filenames like `Is there published psychoacoustic or audio enginee.md` are hard to browse and break in some tools; content is valuable.
3. **Flat `docs/`:** Many unrelated topics (architecture, specs, research, archive) live in one flat list.
4. **Cross-references:** Some files point at `design-spec-files/`; those paths would need updating if we move things.

---

## Proposed structure (ombic-compressor)

```
docs/
├── README.md                    # Index: what lives where (this plan, in short)
├── architecture/                # How the system works, data flow, repos
│   ├── ARCHITECTURE.md
│   ├── WORKFLOW_TWO_REPOS.md
│   ├── mvp_usage.md
│   ├── CURVE_DATA_AND_IMPACT.md
│   ├── CURVE_ANALYZER_IMPROVEMENTS.md
│   ├── CURVE_GRID_SAMPLING_THEORY.md
│   ├── emulation_evaluation.md
│   └── juce_port_handoff.md
├── specs/                       # Product & technical specs (source of truth)
│   ├── GUI_SPEC.md
│   ├── TESTING_PROTOCOL.md
│   ├── PWM_IMPLEMENTATION_ARCHITECTURE.md
│   ├── PWM_MODULE_TECHNICAL_SPEC.md
│   ├── UX_IMPROVEMENTS_SPEC.md
│   ├── DESIGN_IMPROVEMENTS_BACKLOG.md
│   ├── NEON_VISUAL_NEXT_STEPS.md
│   ├── OMBIC_SIDECHAIN_FILTER_MODULE_SPEC.md   # move from repo root
│   ├── OMBIC_COMPRESSOR_JUCE_SPEC.md           # from design-spec-files
│   ├── OMBIC_COMPRESSOR_JUCE_GUIDE.md
│   ├── OMBIC_COMPRESSOR_V2_PRODUCT_GUI_SPEC_OUTLINE.md
│   ├── NEON_BULB_GUI_SPEC.md
│   ├── SPEC_VS_JUCE_GAPS.md
│   └── UX_ASSESSMENT.md
├── research/                    # Perplexity & other research (keep, rename for clarity)
│   ├── VCA_MODE_PERPLEXITY_RESEARCH.md
│   ├── VCA_VS_PWM_MODES.md
│   ├── PERPLEXITY_GRAPHICS_RECOMMENDATIONS.md
│   ├── PERPLEXITY_PROMPT_NEON_BULB_VISUAL.md
│   ├── GUI_BEST_PRACTICES_RESEARCH.md
│   ├── GUI_DESIGN_ANALYSIS.md
│   ├── neon_bulb_saturation_research_spec.md
│   ├── psychoacoustic_pwm_ultrasonic_research.md   # was "Is there published psychoacoustic or audio enginee.md"
│   └── juce_lookandfeel_draw_override.md            # was "How do developers override LookAndFeel..."
├── design/                      # Mockups and visual reference
│   ├── mockups/
│   │   ├── main-vu-design-options.html
│   │   ├── main-vu-toggle-demo.html
│   │   ├── neon-bulb-mockup.html
│   │   ├── ombic-compressor.html
│   │   └── v2-main-view-as-tube-mockup.html
│   └── (optional: keep design-only notes here)
├── archive/                     # Historical / one-off
│   ├── ARCHIVE_ORIGINAL_VST_REPO.md
│   ├── GITHUB_COMMIT_AND_PUSH.md
│   └── v1_vs_v2_comparison.md
```

**Root:** Keep `README.md`, `OMBIC_SIDECHAIN_FILTER_MODULE_SPEC.md` → move to `docs/specs/`.  
**design-spec-files/:** Contents moved into `docs/specs/`, `docs/research/`, `docs/design/mockups/`; then remove the folder.  
**style/:** Keep as-is at repo root (`style/OMBIC_SOUND_STYLE_GUIDE.md`) — it’s brand, not “docs” in the same sense.

---

## Renames for Perplexity exports (research/)

| Current (design-spec-files or docs) | New name (docs/research/) |
|------------------------------------|---------------------------|
| Is there published psychoacoustic or audio enginee.md | psychoacoustic_pwm_ultrasonic_research.md |
| How do developers override LookAndFeel_V4__drawRot.md | juce_lookandfeel_draw_override.md |

Add a one-line note at the top of each renamed file: `<!-- Original Perplexity prompt: "Is there published psychoacoustic..." -->` (or similar) so the source prompt is traceable.

---

## Cross-reference updates

After moving:

- **README.md:** Point to `docs/architecture/ARCHITECTURE.md`, `docs/specs/GUI_SPEC.md`, `docs/specs/TESTING_PROTOCOL.md` (and any other top-level doc links).
- **Plugin/README.md:** `../docs/GUI_SPEC.md` → `../docs/specs/GUI_SPEC.md`; fonts README → `../docs/specs/OMBIC_COMPRESSOR_JUCE_SPEC.md`.
- **docs/ARCHITECTURE.md** (once in architecture/): Fix internal links to WORKFLOW_TWO_REPOS, CURVE_DATA_AND_IMPACT.
- **docs/neon_bulb_saturation_research_spec.md** (→ research/): Update reference to Perplexity “Documented or measured power spectral density (PSD…” file if that file is also moved/renamed (e.g. into research/ with a short name).
- **NEON_BULB_GUI_SPEC.md:** Change `design-spec-files/neon-bulb-mockup.html` → `docs/design/mockups/neon-bulb-mockup.html` (or relative path from new location).

---

## VST workspace (parent) and ombic-vst-inspector

- **VST/docs/** and **VST/Perplexity/:** Same idea: move Perplexity exports into a `docs/research/` (or `docs/perplexity/`) with renamed files; keep one doc root. Can be a separate follow-up so this PR stays scoped to ombic-compressor.
- **ombic-vst-inspector:** Has its own docs; no change in this plan unless you want the same layout there later.

---

## Implementation order

1. Create `docs/architecture/`, `docs/specs/`, `docs/research/`, `docs/design/mockups/`, `docs/archive/`.
2. Move files into the new dirs (no renames yet); fix any broken internal links.
3. Rename Perplexity exports in `design-spec-files/` and `docs/` to short names; move into `docs/research/`; add “Original prompt” line.
4. Move remaining `design-spec-files/` content (specs + HTML) into `docs/specs/` and `docs/design/mockups/`.
5. Update all cross-references (README, Plugin/README, Plugin/fonts/README, ARCHITECTURE, NEON_BULB_GUI_SPEC, etc.).
6. Remove empty `design-spec-files/`; move root `OMBIC_SIDECHAIN_FILTER_MODULE_SPEC.md` → `docs/specs/` and update link from README.
7. Add `docs/README.md` as an index: one paragraph per folder and “start here” links.

---

## Optional: minimal change

If you prefer minimal churn for now:

- **Keep** `docs/` and `design-spec-files/` as-is.
- **Only:** Rename the two long Perplexity filenames in `design-spec-files/` to `psychoacoustic_pwm_ultrasonic_research.md` and `juce_lookandfeel_draw_override.md`, and add `docs/README.md` that lists what’s in `docs/` vs `design-spec-files/` and recommends “specs in design-spec-files, architecture in docs”.

Then do the full reorganization in a later pass. The plan above is the “clean” end state.
