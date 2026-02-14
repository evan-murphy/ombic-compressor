# MVP Chain Usage

**Purpose:** Single entry point for the emulation MVP: FET or Opto compressor (from measured data), optional FR/THD character (EQ + saturation from analyzer data), optional neon-style tape saturation, with saturator before or after the compressor.

---

## What’s in place

| Piece | Status |
|-------|--------|
| **FET mode** | `output/fetish_v2/` → transfer curve + **envelope** (one-pole attack/release from timing.csv when `attack_param`/`release_param` passed). |
| **Opto mode** | `output/lala_v2/` → transfer curve (peak reduction 0–100%); fixed envelope. |
| **Envelope smoothing** | When timing data exists and FET `attack_param`/`release_param` are set, GR is smoothed with one-pole (time constants from timing table). |
| **FR character** | `character_fr=True` → linear-phase FIR from `frequency_response.csv`. |
| **THD character** | `character_thd=True` → tanh saturation with drive from `thd_vs_level.json`. |
| **Neon saturator** | `NeonTapeSaturation`: stochastic gain modulation + optional tanh after. |
| **Before/after compressor** | `neon_position="before"` or `"after"` in chain. |
| **Unified API** | `MVPChain(mode="FET"|"Opto", ...)` + `process(audio, threshold, ...)`. |

---

## Usage

```python
from emulation import MVPChain
import numpy as np

sr = 48000

# FET: threshold (dB), ratio; pass attack_param/release_param (plugin units) for envelope from timing.csv
chain_fet = MVPChain("FET", sr)
out_fet = chain_fet.process(audio, threshold=-18, ratio=4, attack_param=410, release_param=575)

# Opto: threshold = peak reduction 0–100 (e.g. 25, 50, 75)
chain_opto = MVPChain("Opto", sr)
out_opto = chain_opto.process(audio, threshold=50)

# FET with FR/THD character (EQ + saturation from analyzer data)
chain = MVPChain("FET", sr, character_fr=True, character_thd=True, character_thd_mix=0.5)
out = chain.process(audio, threshold=-18, ratio=4, attack_param=410, release_param=575)

# Neon on, after compressor (default)
chain = MVPChain("FET", sr, neon_enable=True, neon_position="after", neon_dry_wet=0.5)

# Neon before compressor
chain = MVPChain("Opto", sr, neon_enable=True, neon_position="before", neon_depth=0.02)

# Custom data dirs (if not using output/fetish_v2 and output/lala_v2)
chain = MVPChain("FET", sr, fetish_data_dir="path/to/fetish_v2")
chain = MVPChain("Opto", sr, lala_data_dir="path/to/lala_v2")
```

---

## Parameters

**MVPChain(mode, sample_rate, …)**

- **mode:** `"FET"` or `"Opto"`.
- **fetish_data_dir / lala_data_dir:** Override analyzer output dirs (default `output/fetish_v2`, `output/lala_v2`).
- **neon_enable:** Use neon tape saturation in chain.
- **neon_position:** `"before"` = neon → compressor; `"after"` = compressor → neon.
- **character_fr / character_thd:** Enable FR EQ and/or THD saturation from analyzer data.
- **character_fr_drive_db:** Drive level for FR curve (default: first in table).
- **character_thd_mix:** 0–1 mix for THD saturation.
- **neon_depth, neon_modulation_bandwidth_hz, neon_burstiness, neon_g_min, neon_dry_wet, neon_saturation_after, neon_seed:** Passed to `NeonTapeSaturation`.

**process(audio, threshold, ratio=None, attack_ms=None, release_ms=None, attack_param=None, release_param=None, block_size=512)**

- **FET:** threshold (dB), ratio; **attack_param** / **release_param** (plugin units, e.g. 410, 575) enable envelope smoothing from timing.csv.
- **Opto:** threshold = peak reduction 0–100; ratio/attack/release ignored.

---

## What still isn’t in the MVP

- **VST3/AU:** Python only; port to JUCE (or similar) for a loadable plugin.
