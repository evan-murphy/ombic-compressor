# Perplexity Graphics Research — What to Borrow for OMBIC

Review of **Perplexity/What published research, GDC_ADC talks, or technic.md** (five Q&A sections on real-time visualization, transfer curves, metering, GPU rendering, and vintage-emulation visuals). Below: which approaches to borrow for the OMBIC compressor GUI, given your current JUCE stack and OMBIC style.

---

## 1. Real-time visualization (FabFilter, TDR, DMG, others)

**What the doc says:** Transfer curve + level display; multi-trace history (input/output/GR); peak GR labels; optional drawable curves; S-curves and soft knee; detector attribution (which detector is active).

**Suggest borrowing:**

| Approach | Fit for OMBIC | Notes |
|----------|----------------|--------|
| **Transfer curve with live tracer** | ✅ Already have | You have static curve + red dot (current in/out). Doc reinforces this as standard; no change needed. |
| **Scrolling GR history** | ✅ High value | “Moving history of input level and gain reduction” (DDMF Comprezzore, FabFilter) — you flagged this as optional in GUI_DESIGN_ANALYSIS; Perplexity supports doing it. |
| **Peak GR labels / temporal stats** | ⚪ Optional | “Peak gain-reduction labels” (Pro-L 2), “loudness distribution” (TBProAudio Impress). Nice extra; not critical for v1. |
| **Knee / S-curve on transfer curve** | ⚪ Later | LALA/FETish use hard knee today; if you add soft knee in DSP, draw curved transition (TDR-style) in TransferCurveComponent. |
| **Drawable curve / multi-segment** | ❌ Skip for now | DMG/Comprezzore-style; big scope and not in current feature set. |
| **Detector attribution** | ❌ Skip | Single detector per mode; no “which of three detectors is active” to show. |

**Summary:** Prioritize **scrolling GR (and optionally input) history** alongside the existing transfer curve and meters. Consider **peak GR label** (e.g. “Peak: -6 dB”) as a small addition.

---

## 2. Best practices for real-time transfer curve rendering (Pro-C 2, Kotelnikov, JUCE)

**What the doc says:** Curve updates on parameter change (not per-sample); live tracer from smoothed detector; logarithmic dB axes (-60…0); optional knee toggle; mouse-drag on curve to adjust params; Path/bezier for curves; atomics/FIFO + timer poll; 60 FPS target; &lt;1 ms UI–audio latency.

**Suggest borrowing:**

| Approach | Fit for OMBIC | Notes |
|----------|----------------|--------|
| **Curve on param change, tracer from detector** | ✅ Already have | You redraw curve from APVTS and use atomics for the dot; matches. |
| **Log dB axes (-60…0)** | ✅ Already have | TransferCurveComponent uses -50…0; close enough; -60 is fine if you want more headroom. |
| **Path / vector curves** | ✅ Already have | juce::Path with lineTo; doc suggests quadraticBezier for knees when you add soft knee. |
| **Atomics + timer, no locks in processBlock** | ✅ Already have | Document this in code or GUI_SPEC so it’s explicit “best practice.” |
| **60 FPS for curve/meters** | ⚪ Upgrade | You use 25 Hz; doc and next section suggest 60 for “silky” feel. Consider `startTimerHz(60)` for editor/meters and keep smoothing coeff so ballistics stay ~300 ms. |
| **Knee toggle in UI** | ⚪ When you add knee | Only if DSP gains a knee parameter; then “show knee” button to avoid clutter. |
| **Mouse-drag on curve to adjust threshold/ratio** | ⚪ Later | Pro-C 2–style; improves workflow but needs hit-test and param mapping; good v2 feature. |

**Summary:** Align with doc by (1) optionally raising meter/curve refresh to **60 Hz** while keeping VU ballistics, and (2) **documenting** your data flow (atomics + timer, no processBlock locks). Defer interactive curve dragging and knee toggle until you have knee in DSP.

---

## 3. Perceptual effectiveness (VU vs bar vs transfer curve; skeuomorphic vs flat)

**What the doc says:** VU good for smooth/opto GR; bars better for peaks/transients; transfer-curve tracer best for cause–effect (input → GR). Flat/modern UIs win on clarity, scaling, and density; skeuo for familiarity/nostalgia.

**Suggest borrowing:**

| Approach | Fit for OMBIC | Notes |
|----------|----------------|--------|
| **Bar GR + numeric readout** | ✅ Already have | Matches “bars/curves better for digital workflows.” |
| **VU-style ballistics on GR** | ✅ Already have | Fits “smooth, averaged GR” and opto (LALA); keep. |
| **Transfer curve + tracer** | ✅ Already have | “Superior causality understanding”; you’re good. |
| **Flat/modern with clear hierarchy** | ✅ Already have | OMBIC is flat, card-based, not skeuo; doc supports this choice. |
| **Avoid clutter** | ✅ Already have | Grouped sections, optional knee toggle later if needed. |

**Summary:** No structural changes needed. Your mix of **VU ballistics + bar + transfer curve** and **flat OMBIC style** matches the doc’s recommendations.

---

## 4. GPU-accelerated rendering (JUCE OpenGL, VSTGUI, iPlug2, ImGui)

**What the doc says:** GPU for vector paths, blurs, scrolling histories; 60 FPS min; &lt;1 ms UI–audio latency; ring-buffer textures for history; JUCE OpenGL for rich UIs; 100–500 µs/frame for “rich” JUCE UI.

**Suggest borrowing:**

| Approach | Fit for OMBIC | Notes |
|----------|----------------|--------|
| **60 FPS target** | ⚪ Optional | If you bump timer to 60 Hz, you’re in line with “60 FPS min” without touching GPU. |
| **&lt;1 ms UI–audio latency** | ✅ Already in range | Atomics + timer; no heavy work on audio thread. |
| **Ring buffer for history** | ✅ When you add GR history | Use a ring buffer of GR (and optionally input) samples for the strip; no GPU required for a simple path. |
| **JUCE OpenGL / GPU** | ❌ Defer | Doc says JUCE can use OpenGL for “smooth splines, HiDPI”; your current CPU Path drawing is fine unless you see frame drops or need 4K+. |
| **Blurs/glows** | ❌ Skip | OMBIC style is hard shadows, no blur; keep it. |

**Summary:** **Don’t** add GPU/OpenGL for now. **Do** use a **ring buffer** when you implement GR history, and consider **60 Hz** timer if you want snappier meters/curve.

---

## 5. Vintage hardware emulation visuals (UAD, Softube, Plugin Alliance)

**What the doc says:** Skeuomorphic panels, THD meters, VU needles, spectrum overlays for harmonics, transformer/drive selectors.

**Suggest borrowing:**

| Approach | Fit for OMBIC | Notes |
|----------|----------------|--------|
| **VU-style ballistics** | ✅ Already have | You already chose “smooth” metering; doc supports that for opto/saturation contexts. |
| **Dedicated “saturation”/drive feedback** | ⚪ Optional | Neon saturator could expose a simple “drive amount” or level meter (no need for full THD); low priority. |
| **Skeuomorphic knobs/needles** | ❌ Skip | OMBIC is flat and modern; doc says flat is preferred for density and scaling. |
| **Spectrum / harmonic overlay** | ❌ Skip | Out of scope for a compressor/saturator focused on dynamics and curve. |

**Summary:** Keep OMBIC **flat and modern**. Your existing **VU-style ballistics** are the main borrow from this section; no need for vintage-style visuals or THD/spectrum.

---

## Priority summary

1. **Do consider (high impact, reasonable effort)**  
   - **Scrolling GR history** (and optionally input): ring buffer in editor, draw path or strip; aligns with FabFilter/DDMF and your optional item in GUI_BEST_PRACTICES_RESEARCH.  
   - **60 Hz** for editor/meter/curve timer (keep ballistics ~300 ms with adjusted smoothing if needed).

2. **Do consider (small polish)**  
   - **Peak GR label** (e.g. “Peak: -X dB” with short hold/decay).  
   - **Document** atomics + timer + no processBlock locks in GUI_SPEC or code comments.

3. **Later / when DSP supports it**  
   - **Soft knee** in transfer curve (quadraticBezier), plus optional knee toggle.  
   - **Interactive curve** (drag to change threshold/ratio).

4. **Skip for now**  
   - GPU/OpenGL, drawable curves, detector attribution, skeuomorphic/vintage visuals, THD/spectrum, blurs.

I can next turn any of the “Do consider” items into a short implementation plan (e.g. GR history component + 60 Hz timer) or leave this as reference only.
