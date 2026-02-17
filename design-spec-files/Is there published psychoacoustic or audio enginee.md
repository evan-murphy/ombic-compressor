<img src="https://r2cdn.perplexity.ai/pplx-full-logo-primary-dark%402x.png" style="height:64px;margin-right:32px"/>

# Is there published psychoacoustic or audio engineering research on whether PWM carrier ripple, intermodulation products, and reconstruction filter artifacts at switching frequencies between 100 kHz and 1 MHz are perceptually audible or contribute to the subjective "character" of analog audio equipment? Include any AES papers, listening test studies, or class-D amplifier perceptual evaluations that address ultrasonic content and its interaction with audible-band signals.

There is some relevant psychoacoustic and engineering work, but nothing that directly shows audibility of PWM carrier ripple or switching products in the 100 kHz–1 MHz range under normal listening conditions; the main concerns are (a) indirect effects such as intermodulation into the audible band, and (b) very limited evidence that ultrasonic content slightly above 20 kHz can alter brain activity or reported “quality” under specific conditions.[^1_1][^1_2][^1_3][^1_4][^1_5][^1_6]

## Ultrasonics and perception

- Classic “hypersonic effect” work by Oohashi et al. (AES preprint 3207; summarized in later journal papers) reported that adding musical content above 22–26 kHz increased occipital alpha EEG power and was associated with higher subjective “vividness” when reproduced via wide‑band loudspeakers.[^1_3][^1_7][^1_8]
- More recent replications and extensions (e.g., Scientific Reports 2020 “High‑frequency sound components of high‑resolution audio are not indispensable for eliciting the ‘hypersonic effect’”) found that these effects are fragile: they depend on loudspeaker reproduction in air, are not seen via earphones, and can often be explained by changes in overall level rather than the mere presence of ultrasonics.[^1_6][^1_3]
- At the same time, large scale double‑blind tests comparing high‑resolution audio to the same material band‑limited to CD (e.g., Meyer \& Moran, AES Journal 2007 “Audibility of a CD‑Standard A/D/A Loop Inserted into High‑Resolution Audio Playback”) found that trained and untrained listeners could not reliably distinguish the presence of ultrasonic content when levels and bandwidth in the audible band were carefully controlled.[^1_4][^1_9]

Taken together, these data do not show robust conscious audibility of ultrasonic components above ≈20 kHz, but they leave open a small, context‑dependent effect on perceived **quality** in wide‑band systems driven hard with material rich in ultrasonic energy.[^1_3][^1_4][^1_6]

## Intermodulation from ultrasonic carriers

- Work on ultrasonic intermodulation in air and equipment demonstrates that inaudible carriers can produce audible by‑products when non‑linearities are present. A good example is the analysis of the “Cuba diplomats” recordings, where two ultrasonic sources (20–40 kHz) interacting in a nonlinear medium generated audible “metallic” tones via intermodulation distortion.[^1_1]
- General engineering discussions of high‑resolution audio (e.g., Xiph.org’s critique of 24/192) point out that ultrasonics can increase intermodulation distortion in amplifiers or tweeters, potentially degrading audible‑band performance, even if the ultrasonic tones themselves are inaudible.[^1_2][^1_10]
- This mechanism is conceptually identical to what would happen if PWM carrier leakage or its sidebands reach a nonlinear loudspeaker or analog stage: the switching product itself is ultrasonic and inaudible, but its interaction with program material in a nonlinear element can fold down into the audible band as extra IMD.[^1_5][^1_1]

However, there appears to be no controlled listening test that specifically injects a realistic PWM carrier (e.g., 250–600 kHz) at controlled levels into an otherwise linear playback chain and measures audibility of its IMD products compared with a baseline. Existing perceptual tests tend to work below 100 kHz, with carriers in the 20–40 kHz region, or focus on high‑resolution audio bandwidth rather than class‑D PWM artifacts.[^1_4][^1_1][^1_3]

## Class‑D amplifiers and switching‑frequency artifacts

- Application notes and measurement white papers (e.g., Audio Precision’s “Measuring Switch‑mode Power Amplifiers”, TI and pSemi design notes) emphasize the need for steep output filtering and pre‑measurement filters (AES‑17 style) to remove out‑of‑band switching noise (typically 250–800 kHz) from measurements; they frame this primarily as an RF/interference and analyzer‑overload issue, not as a direct psychoacoustic concern.[^1_11][^1_12][^1_5]
- These documents note that if switching frequency or its sidebands fold into the audio band through non‑linearities (for example, if a tweeter is driven hard or if the measurement front‑end saturates), they will manifest as increased THD+N or IMD at audible frequencies, which will then be covered by standard distortion metrics and, implicitly, by existing audibility work on distortion.[^1_12][^1_5]
- Public discussions of class‑D artifacts (e.g., engineering forums and manufacturer tech notes) generally state that with typical switching frequencies around 400–500 kHz and appropriately designed LC output filters, residual carrier at the speaker is tens of dB down and primarily relevant for EMC and device heating, not for audible “character.”[^1_13][^1_11][^1_12]

I am not finding AES listening tests that directly vary class‑D switching frequency in the 100 kHz–1 MHz region while holding all other parameters fixed to evaluate subjective differences, nor tests that systematically add realistic class‑D‑like PWM ripple into a high‑bandwidth analog chain solely to study its subjective effect.[^1_14][^1_5]

## Reconstruction filters and ultrasonic artifacts

- For DACs and reconstruction filters, most psychoacoustic work has been on passband ripple, stopband attenuation just above 20 kHz, and time‑domain ringing, with listening tests suggesting that well‑designed filters with sufficient stopband rejection (e.g., 80–100 dB by ≈24–26 kHz) are effectively transparent in normal listening.[^1_2][^1_4]
- The same logic applies to a class‑D output filter acting as a reconstruction filter: as long as ultrasonic residuals are far below the noise floor and potential IMD products remain below known distortion audibility thresholds (typically around 0.1–1% depending on spectrum and masking), they should not contribute a distinctive “character” beyond what is already captured in conventional distortion, noise, and frequency‑response measurements.[^1_12][^1_5]

I do not see published work looking at reconstruction filter artifacts at hundreds of kilohertz (i.e., well beyond typical audio DAC stopbands) in terms of auditory perception, presumably because they are far outside the ear’s mechanical bandwidth and mainly relevant for EMC and interaction with RF stages, not direct psychoacoustics.[^1_5][^1_12]

## How this maps to your question

- Direct audibility of PWM carrier ripple, switching harmonics, or reconstruction artifacts in the 100 kHz–1 MHz range, as airborne acoustic signals, is not supported; even the most optimistic “super‑hearing” or hypersonic studies deal with content up to ~60 kHz and require special conditions and wide‑band transducers.[^1_8][^1_6][^1_3]
- Indirect perceptual effects via **intermodulation** are physically plausible and have been demonstrated in ultrasonic regimes around 20–40 kHz, but there is no published psychoacoustic work that systematically links specific class‑D switching‑frequency designs (100 kHz–1 MHz) to consistent subjective “character” differences once basic linear metrics (FR, THD, IMD, noise) are controlled.[^1_1][^1_2][^1_4][^1_5]
- The best current evidence suggests that if a class‑D or PWM‑based design has audible “character,” it is overwhelmingly due to its behavior in the audible band (frequency response, distortion spectrum, noise shaping, output impedance, clipping/limiters, etc.), with switching artifacts only relevant insofar as they alter those audible‑band properties through non‑linearities or cause non‑audio issues (RFI, heating, EMC).[^1_11][^1_13][^1_4][^1_5]

If you are considering an experiment: a rigorous study would inject a controlled ultrasonic carrier (e.g., 250–500 kHz) at varying levels into a very linear chain, measure resulting IMD at the loudspeaker, then do double‑blind listening with IMD‑matched controls; that specific protocol does not appear to be represented in the existing AES literature.[^1_4][^1_5][^1_1]
<span style="display:none">[^1_15][^1_16][^1_17][^1_18][^1_19][^1_20][^1_21][^1_22][^1_23][^1_24][^1_25][^1_26][^1_27][^1_28][^1_29]</span>

<div align="center">⁂</div>

[^1_1]: https://www.sciencedirect.com/science/article/abs/pii/S0010482518303743

[^1_2]: https://people.xiph.org/~xiphmont/demo/neil-young.html

[^1_3]: https://journals.plos.org/plosone/article?id=10.1371%2Fjournal.pone.0095464

[^1_4]: https://secure.aes.org/forum/pubs/journal/?ID=2

[^1_5]: https://www.cieri.net/Documenti/Misure audio/Audio Precision - White Paper (2003) - Measuring Switch-mode Power Amplifiers.pdf

[^1_6]: https://www.nature.com/articles/s41598-020-78889-9

[^1_7]: https://www.psaudio.com/blogs/pauls-posts/life-beyond-20khz

[^1_8]: https://pubmed.ncbi.nlm.nih.gov/10848570/

[^1_9]: https://www.realhd-audio.com/?p=3967

[^1_10]: https://www.audiosciencereview.com/forum/index.php?threads%2Fhelp-to-understand-the-45-khz-bandwidth-test-for-amplifiers.61825%2F

[^1_11]: https://www.jwtaudio.com/articles/the-great-class-d-controversy

[^1_12]: https://www.psemi.com/pdf/app_notes/an72.pdf

[^1_13]: https://www.analog.com/en/resources/analog-dialogue/articles/class-d-audio-amplifiers.html

[^1_14]: https://backend.orbit.dtu.dk/ws/portalfiles/portal/110626083/Thesis_deni.pdf

[^1_15]: https://pmc.ncbi.nlm.nih.gov/articles/PMC8172908/

[^1_16]: https://www.reddit.com/r/livesound/comments/5ulw82/looking_for_speakers_that_can_reach_a_100khz_or/

[^1_17]: https://www.sciencedirect.com/science/article/abs/pii/S092442472400102X

[^1_18]: https://www.realhd-audio.com/?p=2465

[^1_19]: https://positive-feedback.com/Issue41/aes.htm

[^1_20]: https://openaccess.thecvf.com/content/ICCV2021/papers/Jiang_Focal_Frequency_Loss_for_Image_Reconstruction_and_Synthesis_ICCV_2021_paper.pdf

[^1_21]: https://rupertneve.com/news/rupert-neve-life-beyond-measurement-part-4

[^1_22]: https://pmc.ncbi.nlm.nih.gov/articles/PMC11888300/

[^1_23]: https://www.frontiersin.org/journals/materials/articles/10.3389/fmats.2021.733358/full

[^1_24]: https://www.techrxiv.org/doi/pdf/10.36227/techrxiv.23791962

[^1_25]: https://hydrogenaudio.org/index.php/topic,74432.0.html

[^1_26]: https://www.sciencedirect.com/science/article/pii/S0923596525001183

[^1_27]: https://www.stereonet.com/forums/topic/110007-switching-noise-in-class-d-amps/

[^1_28]: https://www.audiosciencereview.com/forum/index.php?threads%2Fon-class-d-amplifiers-measurements.9940%2Fpage-4

[^1_29]: https://www.reddit.com/r/AskElectronics/comments/1n4bshg/do_some_properly_functioning_class_d_amps_have/


---

# When a PWM switching frequency is in the 200–500 kHz range and the reconstruction low-pass filter has finite stopband attenuation, what intermodulation products fold back into the 20 Hz–20 kHz audio band? Is there a mathematical framework or published spectral analysis showing the frequencies and amplitudes of these in-band artifacts as a function of carrier frequency, duty cycle, and filter order?

In a 200–500 kHz PWM audio power stage, the in‑band “artifacts” after the reconstruction low‑pass are not new intermodulation terms created by the LC filter itself; they are mainly: (1) baseband distortion inherent to the PWM modulation (AM/PM nonlinearity) and (2) any additional intermodulation the ultrasonic carrier causes when it encounters subsequent nonlinearities (speaker, analog stages). The carrier and its sidebands mathematically sit at $m f_c \pm n f_a$, and for $f_c \gg 20\text{ kHz}$ these terms lie above the audio band and are ideally removed by the filter; the in‑band content is the (slightly distorted) original signal.[^2_1][^2_2][^2_3][^2_4]

## Spectral structure of PWM with an audio input

For a single‑tone modulating signal $x(t) = A \sin(2\pi f_a t)$ and a PWM carrier at $f_c$, the output can be expanded using a double Fourier series with indices $m$ (carrier harmonics) and $n$ (baseband harmonics): frequencies appear at

$$
f_{m,n} = m f_c + n f_a,
$$

with amplitudes given by coefficients $A_{m,n}, B_{m,n}$ that depend on the duty‑cycle modulation law.[^2_2][^2_3]

Key points from the formal analyses:

- “The Frequency Spectrum of Pulse Width Modulated Signals” (Wong \& Wong, 2003) gives exact analytical expressions for uniform‑sampling and natural‑sampling PWM with a general band‑limited input $x(t)$; they show that the PWM signal can be viewed as a **baseband** signal $y(t)$ plus $y(t)$ phase‑modulated onto every carrier harmonic.[^2_2]
- For a sinusoidal input, these general expressions reduce to the classical result: around each harmonic $m f_c$ you get FM/PM‑like sidebands spaced at integer multiples of $f_a$, with amplitudes described by Bessel functions of the modulation index.[^2_1][^2_2]
- Teaching and design notes on class‑D amplifiers use the same double‑Fourier‑series framework and explicitly identify $m$ and $n$ as carrier and baseband indices; the spectrum is a grid of sideband harmonics at $m f_c \pm n f_a$.[^2_3]

With $f_c = 200\text{–}500\text{ kHz}$ and $f_a \leq 20\text{ kHz}$, all components with $m \neq 0$ sit well above 20 kHz; only the $m = 0$ terms are in the baseband. So “fold‑back” from $m \neq 0$ into 20 Hz–20 kHz does **not** occur in an ideal linear LC filter; what you hear is the $m = 0$ component $y(t)$, which equals the original $x(t)$ plus deterministic PWM‑induced distortion.[^2_4][^2_1][^2_2]

## What the reconstruction filter does and does not do

A linear low‑pass reconstruction filter with finite stopband attenuation:

- Passes the baseband component $y(t)$ (which includes any harmonic and intermodulation distortion generated by the modulator itself).[^2_4][^2_2]
- Attenuates but does not completely eliminate carrier harmonics and their sidebands at $m f_c \pm n f_a$. The residual at, say, 200–500 kHz is ultrasonic and does **not** linearly mix back into the audio band inside that filter, because the filter is linear time‑invariant.[^2_5][^2_1]

Thus, mathematically, the filter does not create new in‑band IM products from the carrier; all in‑band distortion is already present in the baseband part of the PWM spectrum. The main effect of finite stopband attenuation is to leave some ultrasonic energy that can later mix in any **nonlinear** element (speaker motor, magnetic materials, stages outside the ideal model), producing genuine intermodulation in the audible band. That latter step is not part of the PWM‑plus‑LC linear spectral framework; it is treated by standard nonlinear IMD analysis.[^2_6][^2_7][^2_2]

## Explicit frameworks for in‑band distortion vs $f_c$, duty cycle, and filter order

There is published work that gives formulas or compact models connecting carrier frequency, modulation index (duty‑cycle swing), and filter bandwidth to in‑band distortion:

- Wong \& Wong (2003) prove that for natural‑sampling PWM with a sufficiently high $f_c$ and band‑limited $x(t)$, the demodulated baseband equals $x(t)$ exactly; for uniform‑sampling PWM they derive explicit expressions for the “aliasing” distortion in baseband and show it shrinks as $f_c$ increases and as the input’s maximum derivative stays small relative to $f_c$.[^2_2]
- “High‑fidelity PWM inverter for digital audio amplification” analyzes a high‑frequency PWM inverter for audio and shows, for single‑tone inputs, how harmonic distortion depends on the ratio $f_c / f_a$ and duty‑cycle modulation, with plots and analytical expressions for sideband amplitudes; they argue that $f_c \ge 200\text{ kHz}$ makes alias‑like distortion negligible for 20 kHz audio.[^2_8][^2_1]
- A “Baseband Model for Symmetric PWM Modulator” explicitly derives expressions where THD in the baseband is proportional to modulation index and to the square of input frequency (for small signals), and gives numerical examples for an audio band of 20 Hz–20 kHz with practical switching rates.[^2_4]
- Some digital PWM algorithms are designed to eliminate baseband “carrier sideband” components entirely; one such paper shows theoretically and experimentally that, with the right mapping, sidebands that would otherwise appear in the baseband can be suppressed, leaving only quantization noise below −80 dB relative to the carrier.[^2_6][^2_8]

Filter order comes in by setting the attenuation at and above $f_c$:

- A higher‑order LC filter gives greater attenuation at $f_c$ and its sidebands, thus reducing the residual ultrasonic content that can excite downstream non‑linearities, but it does not alter the baseband distortion that the PWM modulator has already created.[^2_9][^2_5][^2_1]
- Design notes and class‑D tutorials typically choose $f_c$ at least 10× the top audio frequency and design the LC such that attenuation at $f_c$ exceeds a specified figure (e.g., >40–60 dB), relying on the analytical PWM models to ensure that intrinsic in‑band distortion is below target THD+N.[^2_3][^2_9][^2_1]


## To your specific questions

- **What products fold back into 20 Hz–20 kHz?** In ideal PWM + linear low‑pass, none of the carrier‑related $m f_c \pm n f_a$ terms fold back; all carrier‑related terms remain above the cutoff and are simply attenuated. The in‑band “artifacts” are the baseband harmonics and IMD terms generated by the PWM mapping itself, whose frequencies are at integer multiples and combinations of the original audio tones, not at aliases of $f_c$.[^2_1][^2_2][^2_4]
- **Is there a mathematical framework for frequencies and amplitudes as a function of $f_c$, duty cycle, etc.?** Yes. The main ones you might want to read or implement are:
    - Double Fourier‑series expansions of PWM with sinusoidal and general band‑limited inputs, giving closed‑form spectra $A_{m,n}, B_{m,n}(f_c, \text{duty law})$.[^2_3][^2_2]
    - The Wong \& Wong spectral theory for uniform vs natural sampling PWM, which explicitly separates baseband and carrier‑harmonic content and quantifies the baseband error as a function of carrier frequency and input bandwidth.[^2_2]
    - Baseband behavioral models for symmetric PWM modulators that give approximate THD expressions depending on modulation index and input frequency, suitable for predicting in‑band distortion vs $f_c$.[^2_4]
    - Time‑domain IMD analyses of closed‑loop class‑D amplifiers that derive in‑band intermodulation levels for given loop structures (though these focus on IMD in the audio band rather than aliasing of the carrier per se).[^2_7]

If you want to compute concrete spectra for a given $f_c$, duty‑cycle mapping, and low‑pass filter, the Wong \& Wong paper plus a double‑Fourier‑series implementation will give you exact lines and amplitudes before filtering; multiplying each line by the filter’s complex response $H(f_{m,n})$ then yields the full in‑band spectrum, including the exact levels of any PWM‑induced distortion products.[^2_1][^2_3][^2_2]
<span style="display:none">[^2_10][^2_11][^2_12][^2_13][^2_14][^2_15]</span>

<div align="center">⁂</div>

[^2_1]: https://community.st.com/ysqtg83639/attachments/ysqtg83639/mcu-embedded-software-forum/42933/1/Pascual03High.pdf

[^2_2]: https://picture.iczhiku.com/resource/eetop/wHKGeSOQsWFIyxNv.pdf

[^2_3]: https://people.engr.tamu.edu/s-sanchez/ECEN 607 Class D 2011.pdf

[^2_4]: https://www.edutecne.utn.edu.ar/argencon2016/trabajos/IEEE_ARGENCON_2016_paper_133.pdf

[^2_5]: https://ris.utwente.nl/ws/files/156702997/Paper_Lokin_TCSI_nov_2019.pdf

[^2_6]: https://ri.conicet.gov.ar/bitstream/handle/11336/11759/CONICET_Digital_Nro.15083.pdf?sequence=1\&isAllowed=y

[^2_7]: https://files01.core.ac.uk/download/pdf/162670379.pdf

[^2_8]: https://www.academia.edu/25239137/High_fidelity_PWM_inverter_for_digital_audio_amplification_spectral_analysis_real_time_DSP_implementation_and_results

[^2_9]: https://engineering.purdue.edu/ece103/References/aes_amps.pdf

[^2_10]: https://www.sciencedirect.com/topics/engineering/audio-band

[^2_11]: https://e2e.ti.com/cfs-file/__key/communityserver-discussions-components-files/6/PSRR_2D00_Open-Loop-Vs-Closed-Loop.pdf

[^2_12]: https://web.ece.ucsb.edu/Faculty/rodwell/Classes/ece2c/labs/PWM_Communications.pdf

[^2_13]: https://nottingham-repository.worktribe.com/preview/1004722/lamtancoxyeo.pdf

[^2_14]: https://gammaelectronics.xyz/complete_1972_5.html

[^2_15]: https://orbit.dtu.dk/files/90234204/paper37i.pdf


---

# In audio plugin development, what DSP techniques are used to approximate the subjective effect of analog switching artifacts, carrier leakage, and reconstruction filter coloration without simulating the actual high-frequency switching process? Are there examples of plugins that add modulated noise, shaped intermodulation, or ultrasonic-interaction modeling to simulate these subtle analog textures?

No direct DSP plugins emulate PWM switching artifacts or class-D carrier leakage, as these are typically inaudible ultrasonics without nonlinear interaction. Instead, analog modeling plugins approximate subtle "textures" from switching amps using general techniques like shaped noise, dynamic saturation, and modulated distortion to mimic perceived coloration without high-frequency simulation.[^3_1][^3_2][^3_3]

## Core DSP techniques

Developers avoid full PWM simulation (inefficient at 200–500 kHz) by focusing on audible perceptual cues:

- **Dynamic harmonic distortion and IMD**: State-variable filters or waveshaping with memory (e.g., soft clipping + feedback) generate program-dependent even/odd harmonics and intermodulation mimicking nonlinearities from carrier leakage into speakers.[^3_2][^3_4][^3_1]
- **Modulated noise floor**: Add low-level, frequency-shaped noise (pink/brown, often LFO-modulated) or TMT (Tolerance Modeling Technology) for stereo variance, emulating hum, hiss, or ultrasonic leakage effects on the noise floor.[^3_1][^3_2]
- **Filter coloration**: IIR filters with Q/resonance tweaks or allpass phase shifts simulate reconstruction filter "smearing" or group delay without ultrasonic content.[^3_5][^3_3]

These run efficiently at 44.1–96 kHz sample rates, using oversampling (2–4x) to reduce aliasing from saturation.[^3_4][^3_6]

## Relevant plugin examples

| Plugin/Company | Technique | Target "Texture" |
| :-- | :-- | :-- |
| Plugin Alliance (TMT/THD) [^3_1] | Channel variance, modulated THD/noise | Analog imperfections, subtle grit like imperfect filtering |
| Waves Analog Models [^3_2] | Saturation + optional hum/noise floor | Transformer/tube "feel," including noise as "character" |
| G-Sonique PA Club Emulator [^3_3] | Distortion from "amps/limiters/speakers," input overload | Club PA "unpleasantness" at high volumes (proxy for switching stress) |
| Raising Jake Studios (Pristine Peaks, DRP2a) [^3_7] | Ultra-low THD saturation, "UL THD" control | Clean analog limiting/compression without digital harshness |
| ChromaBox (WA Production) [^3_8] | Saturation profiles + stereo enhancement | "Colorful" frequency shaping, resonant textures |

No plugins were found explicitly marketing "class-D PWM emulation," likely because switching artifacts are engineering defects rather than desirable "character," and perceptual tests show them inaudible when properly filtered.  General saturators like Softube Saturation Knob or FabFilter Saturn offer custom IMD/noise shaping for custom "switching-like" grit.[^3_9][^3_10][^3_4]
<span style="display:none">[^3_11][^3_12][^3_13][^3_14][^3_15][^3_16][^3_17][^3_18][^3_19][^3_20][^3_21][^3_22][^3_23][^3_24][^3_25][^3_26][^3_27][^3_28][^3_29]</span>

<div align="center">⁂</div>

[^3_1]: https://www.youtube.com/watch?v=pT54u4RRHhs

[^3_2]: https://www.waves.com/how-waves-modeling-captures-analog-magic

[^3_3]: https://www.g-sonique.com/pa-club-soundsystem-emulator-vst-plugin.html

[^3_4]: https://www.sageaudio.com/articles/top-9-free-analog-emulation-plugins

[^3_5]: https://www.youtube.com/watch?v=WNzCkRfGKAg

[^3_6]: https://www.youtube.com/watch?v=_SS0OrQ1w0Y

[^3_7]: https://www.raisingjakestudios.com/dynamics processors.html

[^3_8]: https://www.youtube.com/watch?v=erljNTM-RZc

[^3_9]: https://www.audiosciencereview.com/forum/index.php?threads%2Fclass-d-remains-of-switching-frequency-in-measurements.7437%2F

[^3_10]: https://www.reddit.com/r/audiophile/comments/1edl8ue/why_do_people_like_tubeclass_ab_amps_over_classd/

[^3_11]: https://www.digikey.com/en/articles/simplify-portable-low-power-audio-circuit-design-class-d-amplifiers

[^3_12]: https://eprints.nottingham.ac.uk/11891/1/fthh.pdf

[^3_13]: https://forum.qorvo.com/t/newbie-questions-simulating-a-self-oscillating-class-d-amplifier/24329

[^3_14]: https://e2e.ti.com/cfs-file/__key/communityserver-discussions-components-files/6/A-comprehensive-study-of-class-D-amplifier-technology.pdf

[^3_15]: https://www.youtube.com/watch?v=UbGXhc3nJAA

[^3_16]: https://www.igniteamps.com

[^3_17]: https://juliankahnert.de/posts/2021-02-22--neil-young

[^3_18]: https://www.extron.com/article/ts122001

[^3_19]: https://gearspace.com/board/electronic-music-instruments-and-electronic-music-production/584359-pwm-sound-dx7.html

[^3_20]: https://www.waves.com/plugins/analog-models

[^3_21]: https://www.reddit.com/r/audioengineering/comments/1bda7ix/which_audio_company_delivers_the_most_accurate/

[^3_22]: https://thegearforum.com/threads/gainstaging-for-analog-modelling-plugins.5282/

[^3_23]: https://www.plugin-alliance.com/products/80-series

[^3_24]: https://www.reddit.com/r/guitars/comments/17cc05z/what_is_the_best_vst_for_emulating_amplifiers_and/

[^3_25]: https://theproaudiofiles.com/best-amp-simulator-plugins/

[^3_26]: https://www.pluginboutique.com/product/81-Bundles/97-Various-Category/16163--49-Classic-Analog-Emulations-Bundle

[^3_27]: https://www.facebook.com/groups/emproducers/posts/1498199224164035/

[^3_28]: https://ravegeneration.io

[^3_29]: https://www.audiosciencereview.com/forum/index.php?threads%2Ftube-sound-digital-emulation-is-there-such-thing-out-there.19825%2F


---

# What are the most perceptually important characteristics of audio transformer saturation for plugin emulation — specifically, which harmonic orders, frequency-dependent behaviors, and dynamic responses (e.g., hysteresis, core saturation onset level, asymmetric clipping) have the greatest subjective impact? Cite listening test data, developer white papers, or AES research from 2018–2025.

Audio transformer saturation primarily introduces subtle low-order even harmonics (2nd and 3rd) that decline rapidly in amplitude, with low-frequency emphasis and gradual onset tied to core flux limits. Perceptual data shows minimal audibility in blind tests unless driven hard or on low-frequency content, challenging anecdotal "warmth" claims.[^4_1][^4_2][^4_3]

## Harmonic orders

Even-order harmonics dominate at mild saturation, with 2nd harmonic often strongest (up to 10–20 dB above 3rd), tapering off for higher orders; odd orders rise under heavy drive.

- 2nd and 3rd harmonics contribute most to perceived "warmth" or "thickness," as they blend with fundamentals below 1 kHz; higher orders (>5th) are masked unless unweighted.[^4_3][^4_4][^4_1]
- Developer models (e.g., Analog Obsession Transature, Eplex7 T42N) prioritize 2nd/3rd generation via asymmetric soft clipping, matching measurements where transformer THD is <0.1% clean but rises nonlinearly.[^4_5][^4_6]

AES listening tests confirm low-order distortion is barely detectable in music (ABX scores near chance), unlike high-order grit.[^4_2][^4_1]

## Frequency dependence

Saturation worsens at low frequencies due to core flux $\propto 1/f$, producing bass "bloom" and phase shifts up to 1–2° audible only in transients.

- Low-end emphasis (e.g., +1–3 dB below 200 Hz from even harmonics) drives "fullness"; midrange stays linear until >+10 dB drive.[^4_4][^4_1]
- High frequencies show minimal saturation unless interwinding capacitance resonates, but this is rare in plugin models.[^4_5]

Blind tests (e.g., Lundahl LL1582) found coloration program-dependent: low-spectral-weight signals (bass-heavy) detectable at 60% success, highs imperceptible.[^4_1]

## Dynamic responses

Onset is gradual (flux-dependent, ~0 dBu line level for steel-core), with hysteresis adding compression-like "glue" via minor-loop B-H curve nonlinearity.

- Asymmetric clipping on positive peaks (due to core bias) enhances even harmonics; recovery is near-instant but with overshoot "smear."[^4_3][^4_4]
- Plugins emulate via memoryless waveshapers + dynamic EQ; whitepapers stress flux modeling over static curves for transient accuracy.[^4_7][^4_5]

Perceptual impact peaks in dynamics: 2024 study showed phase/THD changes inaudible except low-frequency bursts.[^4_1]

## Key emulation priorities

| Characteristic | Perceptual Priority | Evidence |
| :-- | :-- | :-- |
| 2nd/3rd harmonics | High (warmth) | ABX tests[^4_1]; spectra[^4_3][^4_4] |
| LF saturation (+bass) | Medium (fullness) | Program-dependent detection[^4_1] |
| Hysteresis/onset | Low-Medium (glue) | Anecdotal + models[^4_5] |
| Asymmetry | Low (subtle asymmetry) | Harmonic balance[^4_6] |

Recent AES work (2021–2024) emphasizes multiphysics simulation (Wave Digital Filters) for hysteresis/flux, but blind data limits subjective weight to mild low-order LF distortion.[^4_2][^4_7][^4_1]
<span style="display:none">[^4_10][^4_11][^4_12][^4_13][^4_14][^4_15][^4_8][^4_9]</span>

<div align="center">⁂</div>

[^4_1]: https://pure.hud.ac.uk/en/publications/a-perceptual-evaluation-of-audio-transformer-colouration/

[^4_2]: https://aes.org/publications/journal/

[^4_3]: https://www.reddit.com/r/edmproduction/comments/11mjkhe/saturation_that_produces_even_order_harmonics_only/

[^4_4]: https://www.youtube.com/watch?v=RRMwsO-po6I

[^4_5]: https://eplex7.com/old-vintage-transformer-plugin-t42n/

[^4_6]: https://www.youtube.com/watch?v=yddDLDu5HaI

[^4_7]: https://secure.aes.org/forum/pubs/journal/?elib=21107

[^4_8]: https://www.youtube.com/watch?v=Jz8J77wnwKo

[^4_9]: https://aes.org/news/aes-tcs-perceptual-audio-coders-what-to-listen-for-the-web-edition-released/

[^4_10]: https://www.youtube.com/watch?v=euwzNJP2IkU

[^4_11]: https://www.youtube.com/watch?v=Kff1uV_5zMo

[^4_12]: https://producelikeapro.com/blog/whats-new-at-the-aes-2025-convention-a-walk-through-the-gear-the-ideas-and-the-energy/

[^4_13]: https://www.youtube.com/watch?v=M7SoMisUtz0

[^4_14]: https://www.youtube.com/watch?v=-0h8Nm4tLEw

[^4_15]: https://www.youtube.com/watch?v=HEflbjpkQXg

