# Metering Integration Checklist: Making It Better for End Users

This checklist ties the [metering improvements](METERING_IMPROVEMENTS_DORROUGH_INDUSTRY.md) to the existing UI and suggests what still helps end users.

---

## Done (integrated)

| Item | Where | Notes |
|------|--------|------|
| **Peak + RMS from processor** | `PluginProcessor` | Input/output peak, RMS, and L/R peak atomics; all level meters can use peak. |
| **Dual ballistics (peak fast/slow release)** | MeterStrip, MainVuComponent, OutputSection level meters | Level bars show peak with 2 s hold line. |
| **GR fast attack / slow release + hold** | MeterStrip, CompressorSection, OutputSection GR | GR tracks compressor action; hold shows recent max. |
| **45 Hz refresh** | All editors, MeterStrip, MainVu, MainViewAsTube, SidechainFilterSection | Snappier, consistent updates. |
| **−18 dB scale mark** | MeterStrip | Reference level on scale. |
| **Tube view aligned** | MainViewAsTubeComponent | Uses same peak and GR ballistics as other meters. |
| **Transfer curve dot** | TransferCurveComponent | Still uses RMS (average) by design — dot = average level on curve. |

---

## Optional: Better for end users

### 1. Explain what they’re looking at (tooltips / copy)

- **Level meters:** e.g. “Peak level (fast response). Thin line = 2 s peak hold.”
- **GR:** e.g. “Gain reduction: how much the compressor is reducing level. Bar and number update quickly.”
- **Scale:** e.g. “−18 dB is a common reference level for mixing.”
- **Transfer curve dot:** e.g. “Current position: average (RMS) input vs output level.”

**Where:** Set `setTooltip(...)` on MeterStrip, OutputSection meters, CompressorSection GR meter, MainVu readouts, and optionally the scale area. One sentence each is enough.

### 2. Stereo L/R in the UI (optional)

The processor already exposes `inputPeakDbL`, `inputPeakDbR`, `outputPeakDbL`, `outputPeakDbR`. To use them:

- **Option A – Two segments in one bar:** In MeterStrip or OutputSection, draw one bar with left half = L, right half = R (or stacked).
- **Option B – Separate L/R meters:** Add a second pair of small bars (e.g. “L” and “R”) next to IN/OUT when stereo.
- **Option C – Toggle:** “Mono (max L/R)” vs “Stereo (L | R)” so users can switch.

**Benefit:** Balance and width at a glance; matches “industry” monitoring.

### 3. Peak vs average (VU) toggle (optional)

- Add a small control (e.g. “Peak” / “VU” or “Pk” / “Avg”) near the main level meters.
- When “VU”, drive the bar from `inputLevelDb` / `outputLevelDb` (RMS) with ~300 ms ballistics instead of peak.
- Default can stay “Peak” for consistency with current behaviour.

**Benefit:** Users who prefer “loudness” (VU) over “transients” (peak) can choose.

### 4. User-facing docs

- **Plugin README or in-app “About”:** Short “Metering” bullet list, e.g.  
  - Level meters show **peak** level with a 2 s **peak hold** (thin line).  
  - **GR** shows gain reduction with fast response.  
  - Scale includes **−18 dB** as a common reference.
- **Manual / FAQ (if any):** One paragraph on what the meters mean and how to use −18 dB for gain staging.

---

## Summary

- **Already integrated:** All main monitoring surfaces use the new peak + GR ballistics, 45 Hz, and −18 dB where relevant; the tube view matches the rest.
- **To make it better for end users:** Add short tooltips, optionally expose stereo L/R and a Peak/VU toggle, and document metering briefly in README or manual.
