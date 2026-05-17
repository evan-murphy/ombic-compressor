# VST Compressor GUI Best Practices – Research Notes

VST3 compressor plugins benefit from intuitive GUIs that clearly visualize dynamic processing for better user control and feedback. ([FabFilter – basic compressor controls](https://www.fabfilter.com/learn/compression/basic-compressor-controls))

---

## GUI Design Basics

- Use a **WYSIWYG-style** workflow for resizable, scalable interfaces with **automatic parameter binding** (e.g. VSTGUI editor or JUCE's layout system).
- **Group related controls** (e.g. threshold and ratio together); break rigid grids with **subtle spacing** for **visual hierarchy**.
- Add **depth** via highlights, shadows, and **mouse-over effects**.
- Prioritize **intuitive layouts** with **prominent knobs/sliders** for key parameters.
- Use a **modern aesthetic** and **custom color palettes** for brand consistency.

*Ref: [JUCE forum – beautiful plugin GUIs](https://forum.juce.com/t/how-do-you-make-beautiful-plugin-guis/8483)*

---

## Gain Reduction Meter

- Display gain reduction with **real-time meters** that show **actual attenuation speed**.
- Show **input/output meters** alongside GR for context.
- Use **classic VU-style ballistics** (e.g. **~300 ms rise / fall**) so feedback on compression amount and timing is readable; this helps users fine-tune threshold, attack, and release.
- Combine with **graphical history views** for multi-band or precise adjustments.
- GR metering is **essential for functional compressor control**.

*Ref: [OBS forum – gain reduction meter for compressor](https://obsproject.com/forum/threads/gain-reduction-meter-needed-for-compressor.163467/)*

---

## Transfer Curve

- **Plot input level (x-axis) vs output level (y-axis)** as a diagonal **1:1 line** that **flattens above threshold** according to ratio, visualizing **knee** and **gain reduction**.
- **Update the curve dynamically** with parameter changes for real-time feedback (e.g. FabFilter Pro-C 2).
- Example: 4:1 ratio means 4 dB of input over threshold becomes 1 dB of output over threshold.
- This graph is **standard in pro plugins** for mapping compression behavior clearly.

*Ref: [Sonible – transfer functions and compression](https://www.sonible.com/blog/transfer-functions-and-compression/)*

---

## Apply to OMBIC

| Practice | Status | Notes |
|----------|--------|--------|
| Group related controls (threshold, ratio, etc.) | Done | CompressorSection groups mode + sliders; SaturatorSection groups drive/tone/mix. |
| Numeric GR readout + bar meter | Done | GR label + vertical bar in CompressorSection and MeterStrip. |
| Meter strip scale (0 / -20 / -40 dB) | Done | Scale labels on MeterStrip. |
| Input / output meters with GR | Done | MeterStrip: In, GR, Out. |
| Resizable, scalable UI | Done | Editor is resizable; layout uses bounds. |
| Custom palette (OMBIC) | Done | OmbicLookAndFeel, red/blue/teal/ink. |
| **VU-style ballistics (~300 ms)** | Done | MeterStrip and GR meter/readout use one-pole smoothing (~300 ms) for readable rise/fall. |
| **Transfer curve (in vs out)** | Done | TransferCurveComponent: plot input vs output, 1:1 (dashed) below threshold, flattening above by ratio; red dot = current in/out. |
| Mouse-over / depth (highlights, shadows) | Done | Section cards: 6px shadow + blue outline on hover (see GUI_DESIGN_ANALYSIS.md). |
| Graphical GR history | Optional | Scroll or strip showing GR over time. |

See **GUI_SPEC.md** for component hierarchy; add a **TransferCurveComponent** and ballistics (e.g. one-pole smoothing on meter/GR values) in a future iteration.
