# JUCE Port Handoff

**Purpose:** Everything the JUCE port should reference to replicate the Python emulation MVP produced in this repo.

---

## 1. Code to Mirror (Python Reference)

| What | Where |
|------|--------|
| **Chain entry point** | `emulation/processor.py` — `MVPChain` (FET/Opto mode, character, neon before/after) and `PyeEmulationChain` |
| **Compressor + envelope** | `emulation/processor.py` — `MeasuredCompressor`: curve cache from compression CSV, `get_attack_release_ms()` from timing CSV, one-pole envelope in `process()` when `attack_param`/`release_param` are set |
| **FR character (EQ)** | `emulation/character.py` — `FRCharacter`: build linear-phase FIR from `frequency_response.csv` magnitude, convolve (mode='same') |
| **THD character (saturation)** | `emulation/character.py` — `THDCharacter`: drive from `thd_vs_level.json` at reference level (-4 dB), tanh saturation + mix |
| **Neon saturator** | `emulation/neon_saturation.py` — `NeonTapeSaturation`: stochastic gain modulation, optional tanh after, dry/wet |
| **Data loading** | `emulation/loader.py` — `load_analyzer_output()`: reads compression CSV, timing CSV, FR CSV, THD JSON; schema/field names define the contract |

---

## 2. Data the Plugin Uses (Baked or Loaded at Init)

| Data | Path (relative to project) | Format |
|------|-----------------------------|--------|
| **FET** | `output/fetish_v2/` | `compression_curve.csv`, `timing.csv`, `frequency_response.csv`, `thd_vs_level.json` |
| **Opto** | `output/lala_v2/` | Same filenames (no `timing.csv`) |

- **Compression:** `input_db`, `output_db`, `gain_reduction_db`, `threshold`, `ratio`, `attack_ms`, `release_ms` — interpolate (threshold, ratio, input_db) → gain_reduction_db, then apply gain = 10^(-GR/20).
- **Timing:** `attack_param`, `release_param`, `attack_time_ms`, `release_time_ms` — interpolate (attack_param, release_param) → time constants for one-pole envelope (attack when GR increases, release when it decreases).
- **FR:** `frequency_hz`, `magnitude_db`, `drive_level_db` — one drive level; magnitude curve → FIR or biquad EQ.
- **THD:** `level_db`, `thd_percent`, `harmonics` — use THD% at one reference level to set tanh drive.

---

## 3. Specs / Behaviour (Produced in This Chat)

| Doc | Purpose |
|-----|--------|
| `docs/mvp_usage.md` | MVP API: `MVPChain(mode, sample_rate, character_fr, character_thd, neon_enable, neon_position, …)`, `process(audio, threshold, ratio, attack_param, release_param)`. FET vs Opto params. |
| `docs/emulation_evaluation.md` | What the analyzer outputs provide and how they map to the emulation. |
| `pye_emulation_spec.md` | Overall emulation design (envelope, gain, character). |
| `docs/measurement_research_refs.md` | Points to Perplexity research on measurement order and conventions. |
| `docs/sampling_tool_v2_gap_analysis.md` | Why v2 order is FR → compression → THD → timing; 63% attack/release. |
| `docs/neon_bulb_saturation_research_spec.md` | Neon block: noise → shaping → gain map → multiply, optional tanh after. |

---

## 4. One-Line Summary for the Other Chat

**"Port the Python emulation in this repo: `emulation/processor.py` (MVPChain + MeasuredCompressor with envelope from timing.csv), `emulation/character.py` (FRCharacter, THDCharacter), and `emulation/neon_saturation.py` (NeonTapeSaturation). Use analyzer data in `output/fetish_v2/` (FET) and `output/lala_v2/` (Opto). Match behaviour and API in `docs/mvp_usage.md` and load data per `emulation/loader.py`."**

---

## 5. Linear vs In-Chat for the Port

**Linear.app** is for project/issue tracking (backlog, sprints, assignees). It does not run or edit code. The JUCE port is implementation work: C++, file I/O, DSP, and plugin/API wiring.

- **Recommendation:** Do the port **in this chat** (or another code editor/agent). Use Linear only if you want to break the work into stories and track progress (e.g. “Data loader done”, “Compressor done”, “Plugin wired”) for visibility or handoff.
- All implementation (loader, compressor, character, neon, chain, processor hookup) can be completed here; Linear is optional for task breakdown and status.

---

## 6. JUCE Port Status (Plugin/Source/Emulation)

The following C++ code mirrors the Python reference and is wired into the plugin processor:

| Component | Location | Notes |
|------------|----------|--------|
| **Data loading** | `Emulation/DataLoader.cpp` | Loads compression_curve.csv, timing.csv, frequency_response.csv, thd_vs_level.json from a directory. |
| **MeasuredCompressor** | `Emulation/MeasuredCompressor.cpp` | Curve cache, `gainReductionDb()`, `getAttackReleaseMs()`, one-pole envelope in `process()` when attack_param/release_param are set. |
| **FRCharacter** | `Emulation/FRCharacter.cpp` | Magnitude from FR CSV → linear-phase FIR (IFFT of magnitude spectrum), convolution in `process()`. |
| **THDCharacter** | `Emulation/THDCharacter.cpp` | THD% at reference level → tanh drive, mix with dry. |
| **NeonTapeSaturation** | `Emulation/NeonTapeSaturation.cpp` | Stochastic gain modulation (noise → LP/HP → gain map → multiply), optional tanh after, dry/wet. |
| **MVPChain** | `Emulation/MVPChain.cpp` | FET or Opto mode, optional FR/THD, neon before/after compressor; `process(buffer, threshold, ratio, attack_param, release_param)`. |

**Data path:** The processor resolves the data root in `ensureChains()`: it tries `getCurrentWorkingDirectory()` then the application directory, and looks for `output/fetish_v2/compression_curve.csv` and `output/lala_v2/compression_curve.csv`. For standalone or DAW use, copy the `output/` tree next to the executable or set the host’s working directory to the project root.
