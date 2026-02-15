# Trash font

Place **Trash-Regular.ttf** and **Trash-Bold.ttf** here for the OMBIC Compressor UI (see `design-spec-files/OMBIC_COMPRESSOR_JUCE_SPEC.md`).

**Embedded (recommended for release):** If these files are present when you build, they are baked into the plugin. No extra setup needed for users.

**Runtime fallback if not embedded:** The plugin looks for Trash in this order:
1. Embedded data (only if you built with the font files in `Plugin/fonts/`)
2. **Plugin/fonts/** — e.g. run from repo with `OMBIC_COMPRESSOR_DATA_PATH` set to the repo root, or run from `Plugin` so `fonts/` is found
3. **style/** — `OMBIC_COMPRESSOR_DATA_PATH/style/` or cwd `style/`
4. **OMBIC_FONT_PATH** — env var pointing at a folder that contains `Trash-Bold.ttf` or `Trash-Regular.ttf`
5. System default font

The project builds with or without these files; the UI uses the first available source above.
