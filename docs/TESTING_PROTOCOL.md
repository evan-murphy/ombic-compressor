# Ombic Compressor – Testing Protocol (REAPER)

Use this checklist to verify the plugin in REAPER after a build. The plugin must load its curve data from disk, so **run REAPER from the project root** (or set your project folder to the VST repo root) so that `output/fetish_v2/` and `output/lala_v2/` are found.

---

## 1. Load the plugin

1. **Start REAPER** from the VST project directory (e.g. `open -a REAPER` from the repo root, or set REAPER’s working directory to the repo).
2. Create or open a project, add a track, and insert **VST3: Ombic Compressor** on that track.
3. Confirm the GUI opens: red “OMBIC Compressor” header, Compressor section (Mode, Threshold, etc.), Neon Saturation section, Output section, and the meter strip (In / GR / Out) on the right.

---

## 2. Basic signal flow

1. **Play material** (loop or live input) so the track has signal.
2. **Bypass** the plugin and note the level; **enable** it and confirm you still hear signal (no mute).
3. With the plugin on, confirm the **In** and **Out** meters move with level; **GR** should move when the compressor is reducing gain (see §4).

---

## 3. Mode switching (Opto vs FET)

1. Set **Mode** to **Opto**. Change **Threshold** (0–100). You should hear more/less compression; GR meter should reflect it.
2. Set **Mode** to **FET**. Adjust **Threshold** (in dB), **Ratio**, **Attack**, **Release**. Adjust **Output** (makeup/trim). Confirm GR and level change as expected.
3. Switch back and forth between Opto and FET and confirm no crashes and that settings hold where applicable.

---

## 4. Visual representation of compression

1. With **FET** and material playing, lower **Threshold** (e.g. to around -24 dB) so the compressor is clearly working.
2. Check:
   - **GR meter** (in the Compressor section and in the right-hand strip) shows gain reduction (e.g. red bar or numeric readout).
   - **Numeric GR readout** (e.g. “-6.2 dB”) updates in real time when present.
3. Raise Threshold so the signal is mostly below threshold; GR should go to 0 (or near 0). Lower it again and confirm GR returns.

---

## 5. Neon Saturation

1. Turn **Neon Saturation** on (if there’s an On control) and set **Mix** to taste (e.g. 0.5–1.0).
2. Increase **Drive** and **Tone** and confirm audible saturation and no crashes.
3. Saturator is fixed **before** the compressor; drive and mix affect how much hits the compressor.

---

## 6. Data path (if something seems wrong)

- If the compressor doesn’t seem to reduce gain in either mode, the plugin may not have found the curve data. Check:
  - Current working directory when REAPER was started (should be the repo root so `output/fetish_v2/` and `output/lala_v2/` exist).
  - No error messages in REAPER’s console (if available).
- Rebuild and re-copy the VST3 to `~/Library/Audio/Plug-Ins/VST3/` if you changed code or data paths.

---

## 7. What to listen for

- **Opto:** Smoother, more “leveling” compression; threshold 0–100 controls amount.
- **FET:** More control (threshold in dB, ratio, attack, release); GR should track the measured FET curve and envelope.
- **Neon:** Extra harmonic character and level modulation; before compressor = more hit on the detector; after = more “finishing” saturation.

---

## 8. GUI best practices (further research)

For VST compressor GUI best practices (gain reduction metering, transfer curve, layout), consider a short **Perplexity** (or similar) search, e.g.:

- **Query:** “VST3 compressor plugin GUI best practices gain reduction meter transfer curve”
- **Focus:** How professional compressors show GR (numeric vs bar, scale), input/output metering, and whether a transfer-curve graph is expected. Use the results to refine the OMBIC GUI (meter scale, GR readout, and any future transfer-curve view) while keeping the existing OMBIC design system.

A few notes are collected in **docs/GUI_BEST_PRACTICES_RESEARCH.md** for when you run that research.
