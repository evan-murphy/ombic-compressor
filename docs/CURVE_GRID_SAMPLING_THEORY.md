# Curve Grid “Sample Rate”: Why 3 Points Feels Small and What Theory Says

**TL;DR:** Yes, increasing grid fidelity (more points per parameter) is justified. The same idea as Nyquist applies in parameter space: you need enough samples to represent the “bandwidth” of the function you’re interpolating. Three points per dimension is the minimum for any interpolation; 5–7+ gives smoother sweeps and smaller error. There is solid academic footing for this.

---

## 1. The analogy: Nyquist in parameter space

In classic **Nyquist–Shannon** sampling:

- A **bandlimited** signal (no energy above some max frequency \(f_{\max}\)) can be **exactly** reconstructed from samples if you sample at rate \(\geq 2 f_{\max}\).
- If you sample **below** that rate, you get **aliasing**: high-frequency content folds back and distorts the result.

For our **curve data**, the “signal” is not audio—it’s the **function** we’re sampling in **parameter space**, e.g.:

- \(f(\text{threshold}, \text{ratio}, \text{input\_db}) = \text{gain\_reduction\_db}\)
- or \(g(\text{attack\_param}, \text{release\_param}) = (\text{attack\_ms}, \text{release\_ms})\).

The analogous idea:

- If this function is **smooth** (bounded rate of change, or “bandlimited” in the parameter dimensions), then a **sufficiently dense** grid of measurements allows accurate **reconstruction by interpolation**.
- If the grid is **too coarse**, we **undersample**: the interpolant can’t follow the true curve, so we get **error** (or perceived “steppiness”), similar in spirit to aliasing.

So: **grid density** in threshold/ratio/attack/release is like **sample rate** in time; more points = higher “parameter-space sample rate” = better representation of the underlying behaviour.

---

## 2. Multidimensional sampling (academic footing)

**Petersen–Middleton theorem** (1962) generalises Nyquist–Shannon to **multidimensional** spaces:

- A **wavenumber-limited** (bandlimited) function in \(\mathbb{R}^n\) can be **perfectly reconstructed** from its values on a **sufficiently fine lattice** of points.
- The lattice must satisfy a condition that prevents the periodic repetition of the spectrum from overlapping (no aliasing in the multidimensional sense).

References:

- Petersen, D.P. & Middleton, D. (1962). “Sampling and Reconstruction of Wave-Number-Limited Functions in N-Dimensional Euclidean Spaces.” *Information and Control*, 5(4), 279–323.
- Wikipedia: [Multidimensional sampling](https://en.wikipedia.org/wiki/Multidimensional_sampling).

So conceptually: our (threshold, ratio, input_db) space is a 3D parameter space; the “signal” is the gain-reduction surface. A fine enough grid + correct interpolation gives a good reconstruction. **3 points per dimension is not “fine enough” for smooth perception**—it’s the bare minimum to define a piecewise-linear behaviour between endpoints.

---

## 3. Linear interpolation error (practical bound)

For **linear interpolation** (what we use between samples), there is a clean error bound:

- For a **twice differentiable** function \(h\), the **maximum linear interpolation error** between two consecutive samples (spacing \(\Delta\)) is bounded by  
  \[
  |e| \leq \frac{M_2}{8} \Delta^2,
  \]  
  where \(M_2\) is the maximum absolute value of the **second derivative** of \(h\) over that interval.

So:

- **Error scales with \(\Delta^2\).** Doubling the number of points (halving \(\Delta\)) roughly **quarters** the worst-case error.
- **Curvature matters.** Where the true curve bends sharply (e.g. around knee, or ratio changing slope), \(M_2\) is large, so you need **more points** there (or accept more error).

Reference:

- Julius O. Smith III (CCRMA), “Linear Interpolation Error Bound,” [Digital Audio Resampling](https://ccrma.stanford.edu/~jos/resample/Linear_Interpolation_Error_Bound.html).

So yes: **increasing “sample rate” (grid density) directly reduces interpolation error** in a predictable way.

---

## 4. What “3” gives you vs what you want

- **3 points per dimension**  
  - Defines **2 intervals**. Linear interpolation can only follow a **straight line** between samples.  
  - Any **curvature** (e.g. soft knee, or ratio changing the slope of the transfer) is **underrepresented**—we’re effectively assuming the function is piecewise linear with one bend per interval.  
  - So 3 is the **minimum** to have “low / mid / high” behaviour, not a density that captures fine detail.

- **5–7+ points per dimension**  
  - More intervals → interpolant can follow **curvature** better; error bound drops with \(\Delta^2\).  
  - Knob sweeps feel **continuous** rather than a few steps.  
  - Aligns with a “2× oversample” rule of thumb: if you think of one “cycle” of meaningful change in threshold as ~6–12 dB, then 2 samples per “cycle” would suggest spacing on the order of 3–6 dB (e.g. 5–7 points over 24 dB range).

So **3 is small**; increasing fidelity is well justified by both the Nyquist-like analogy and the interpolation error bound.

---

## 5. Recommended grid density (for the analyzer)

A practical target, borrowing from sampling theory and the error bound above:

| Dimension        | Current | Minimum (5–7) | **Chosen: 20** (for ~5 modes) | Rationale |
|-----------------|---------|----------------|-------------------------------|-----------|
| **Threshold**   | 3       | 5–7            | **20**                        | Smoother sweep; capture knee; \(\Delta^2\) error reduction. 20 keeps total size fine across 5–10 modes. |
| **Ratio**       | 3       | 5–7            | **20**                        | Ratio strongly affects slope; 20 = smooth ratio sweep. |
| **Input level** | 25      | Keep           | Keep or match                 | Already dense; main issue is threshold/ratio. |
| **Timing (attack × release)** | 5×5 | 5×5 or 7×7 | **20×20 optional** | 5×5 okay if populated; finer if envelope feel matters. |

So:

- **Increase “sample rate” in parameter space** by going to at least **5–7 threshold** and **5–7 ratio** (e.g. FET: threshold -30, -24, -18, -12, -6 dB and ratio 2, 4, 8, 12, 20 or similar).
- That increases curve CSV size (e.g. 5×5×25 = 625 rows per mode instead of 225) but stays manageable and gives a clear improvement in perceived continuity and interpolation error.

---

### 5.1 Design choice: 20 per dimension (for ~5 modes, &lt;10)

If the product goal is **on the order of 5 compression modes** (and certainly fewer than 10), **20 points per dimension** (threshold, ratio) is a good target:

- **Quality:** 20×20 gives very small interpolation error (Δ²) and smooth, continuous-feeling knobs.
- **Total cost:** Per mode, 20×20×25 = 10,000 rows ≈ 550 KB per compression CSV. For 5 modes that’s ~2.75 MB of curve data total; for &lt;10 modes, still well under ~6 MB. So total bundle size stays reasonable.
- **Non-arbitrary:** 20 is consistent with the formula in §7.2 for a strict E_max and moderate M₂, and the “~5 modes” product constraint makes the aggregate size obviously acceptable.

**Recommendation:** Use **N = 20** for threshold and for ratio in the analyzer grid when targeting multiple compression modes (e.g. 5, and &lt;10). Same 20×20 (or 20×20×25 input levels) per mode.

---

## 6. Summary

- **Nyquist-style idea:** In parameter space, a sufficiently fine grid is needed to represent the “bandwidth” of the transfer (and timing) functions; 3 points per dimension is undersampled for smooth, accurate interpolation.
- **Academic support:** Petersen–Middleton extends Nyquist to multidimensional lattices; linear interpolation error is bounded by \(\Delta^2 \cdot M_2/8\) (Stanford CCRMA / Smith).
- **Practical takeaway:** Yes, **fidelity should increase**. For a product with **~5 compression modes (fewer than 10)**, use **20 points per dimension** (threshold, ratio): smooth knobs, low interpolation error, and total curve data (~2.75 MB for 5 modes) stays small. See §5.1 and §7.

See also: **CURVE_DATA_AND_IMPACT.md** for how the current 3×3 grid is used (and where ratio isn’t interpolated yet).

---

## 7. How expensive is a parameter point? Why not 20? 100?

### 7.1 Cost per point (storage and memory)

**Compression curve** is a 2D grid (threshold × ratio) × 1D (input level):

- Each **(threshold, ratio)** combination has **25 rows** (one per input level step). So one “parameter point” in the 2D grid = **25 CSV rows**.
- Rough size per row: ~50–60 bytes (eight numeric fields as ASCII). So **one (threshold, ratio) point ≈ 25 × 55 ≈ 1.4 KB** in the CSV.
- **Total CSV size** (one mode):  
  \(N_{\text{thresh}} \times N_{\text{ratio}} \times 25 \times 55\) bytes ≈ **\(N^2 \times 1.4\) KB** if \(N_{\text{thresh}} = N_{\text{ratio}} = N\).

| N (per dimension) | (threshold × ratio) points | Rows | CSV size (approx) |
|-------------------|----------------------------|------|---------------------|
| 3 | 9 | 225 | ~12 KB |
| 5 | 25 | 625 | ~34 KB |
| 7 | 49 | 1,225 | ~67 KB |
| 10 | 100 | 2,500 | ~137 KB |
| 20 | 400 | 10,000 | ~550 KB |
| 100 | 10,000 | 250,000 | ~14 MB |

**Runtime:** The plugin loads all rows into `compressionRows`, then builds `curveCache_`: one map entry per (threshold, ratio) key, each holding two vectors of 25 floats. So memory is on the order of the CSV size (plus map overhead). Lookup is O(log(curves)) + O(log(25)) — trivial even for 10,000 curves. So **CPU is not the limit; storage and “do we need that many?” are.**

So: **one extra “parameter point” in the 2D grid costs ~25 rows ≈ 1.4 KB on disk and a small fixed amount in RAM.** 20 or 100 per dimension is **affordable** (e.g. 20 → ~0.5 MB, 100 → ~14 MB per CSV). The real question is: **what N do we need so the number isn’t arbitrary?**

---

### 7.2 Choosing N by formula (no arbitrary cap)

Use the **interpolation error bound** and a **target maximum error** to derive spacing, then N.

**1. Pick a target max error \(E_{\max}\) (dB of gain reduction).**  
Example: 0.5 dB — below typical level JND so the user won’t hear interpolation error.

**2. Linear interpolation error (1D in one parameter):**  
\[
|e| \leq \frac{M_2}{8} \Delta^2.
\]  
So  
\[
\Delta \leq \sqrt{\frac{8 E_{\max}}{M_2}}.
\]  
\(M_2\) = max absolute second derivative of gain reduction with respect to that parameter (threshold or ratio) over the range.

**3. Estimate \(M_2\).**  
- For **threshold**: GR vs threshold is roughly linear above the knee; curvature is concentrated near the knee. A reasonable range is \(M_2 \approx 0.05\)–\(0.2\ \text{dB}/(\text{dB})^2\) (e.g. slope change per dB of threshold).  
- For **ratio**: GR vs ratio is often concave; \(M_2\) might be on the order of 0.1–0.5 in “per (ratio unit)²” terms depending on scaling.

**4. Compute spacing and N.**  
- Example (threshold): range 24 dB (-24 to 0), \(E_{\max} = 0.5\ \text{dB}\), \(M_2 = 0.1\):  
  \(\Delta \leq \sqrt{8 \times 0.5 / 0.1} = \sqrt{40} \approx 6.3\ \text{dB}\).  
  Number of intervals = 24 / 6.3 ≈ 3.8 → **4 intervals → N = 5 points.**  
- Stricter (sharper knee, \(M_2 = 0.3\)): \(\Delta \leq \sqrt{8 \times 0.5 / 0.3} \approx 3.7\ \text{dB}\) → 24/3.7 ≈ 6.5 → **7 points.**  
- Same idea for **ratio**: define range (e.g. 2–20), pick \(E_{\max}\) and \(M_2\), then \(\Delta_{\text{ratio}} \leq \sqrt{8 E_{\max} / M_2}\), and \(N_{\text{ratio}} = \lceil \text{range} / \Delta_{\text{ratio}} \rceil + 1\).

So:
\[
\boxed{
N \approx \frac{\text{param range}}{\sqrt{8 E_{\max} / M_2}} + 1
}
\]
**You choose \(E_{\max}\) (e.g. 0.5 dB) and \(M_2\) (from a few measurements or a conservative guess); N is then determined, not arbitrary.**

---

### 7.3 Why not always 20 or 100?

- **Cost:** 20 is still cheap (~0.5 MB per CSV). 100 is larger (~14 MB) but fine for a plugin bundle. So **storage alone doesn’t force a low N.**

- **Diminishing return:** Once interpolation error is **below perceptual threshold** (e.g. &lt; 0.3–0.5 dB), extra points don’t improve what the user hears. The **formula in 7.2** gives the N that achieves your chosen \(E_{\max}\). Going to 20 or 100 **past that** doesn’t hurt, but it doesn’t help perception either — you’re just adding measurement time and file size.

- **Measurement reality:** Hardware and level repeatability have limits. If step-to-step variation is on the order of 0.2–0.5 dB, having 100 points doesn’t mean you “know” the curve to 0.01 dB — the **effective resolution** is capped by noise. So beyond some N (often in the low teens to twenties for a 24 dB range), you’re interpolating between noisy samples. The formula still gives a **principled** N; going much beyond it (e.g. 100) is only justified if you have very low measurement noise and want a safety margin.

**Summary:**  
- **One parameter point** (one (threshold, ratio) cell) ≈ **25 rows ≈ 1.4 KB** (CSV); runtime cost is negligible.  
- **N is not arbitrary:** set \(E_{\max}\) and \(M_2\), then \(N = \lceil \text{range} / \Delta \rceil + 1\) with \(\Delta = \sqrt{8 E_{\max} / M_2}\).  
- **20** is fine if the formula gives ~10–15 and you want headroom; **100** is only worth it if you need error far below perception and have very clean measurements.
