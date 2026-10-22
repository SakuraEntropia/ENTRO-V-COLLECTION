# Hardware detail pass — 2026-10-22

Current branch: main. Local commits only, per user instruction. Preserve the other agent's 99e35af preset/expression changes.

- ONE: oscillator controls now form the hardware's point-up hexagon: octave at top, frequency/beat frequency upper left/right, wave angle/pulse width lower left/right, mix at bottom. OLED at centre shows three periods. Four LFOs occupy the far-left strip; ladder sits below SVF; modulation matrix opens in the centre MOD page. All original 408 IDs/order retained; nine default-zero `one.pN.vcoN.beatHz` controls appended, -7 to +7 Hz, genuinely added to oscillator frequency. Total 417 parameters, 145 exposed per selected part including shared controls.
- Knobs: separate vector geometry for Moog One scalloped black skirts/silver inset/white index dots and octave chickenheads, Matriarch fine-fluted skirts/silver caps/black index lines, PolyBrute black caps/white lines and silver ladder cutoff.
- PolyBrute: existing functional XY/Z MorphPad now in the keyboard-left sculpted wooden recess. Pitch/mod wheels moved into the upper-left panel. Bindings and pressure fallback tested.
- Continuum: 204 parameters; original 128 IDs/order retained. Added linked compressor, complementary tilt EQ, pre/post gains; two generated-IR model/length/tuning/width sets; six separately compiled A-F expressions with transfer curve/offset/scale; six matrix Master columns assigned to 20 master destinations; extended filter modes and 1-4 cascades; 8/48 filter banks with bandwidth spread/spectral centre/weight; free/retrigger/one-shot/gated SG cycles and trigger inputs. Four detail pages keep the wide 16×22 matrix dominant. Expressions save in the FORMULAS state child; older states restore expression defaults. Audio receives compiled formulas via nonblocking snapshot.
- Matriarch original colour scheme, 79 jacks, and functional GLIDE/PLAY/HOLD/TAP preserved.

## Verification

Engine, preset and panel tests passed after the hexagon correction. Tests cover actual constant-Hz beat tuning at two octaves across three parts; three-period OLED repetition and parameter response; hexagon positions; every parameter binding on every detail page; compressor linkage/bypass, tilt frequency response, convolution variants, named formula isolation/invalid-expression rejection, 48 banks, filter cascades, master routing and SG one-shot retrigger. Panel PNG writer now truncates previous files so previews cannot silently show an older image.

Native screenshots are in the current chat outputs/hex-final directory. Release build/installation results are recorded in the delivery report.

## Fidelity limits

Reference-based vector redraws, not measured 1:1 reproductions. Continuum remains an original approximation: generated IRs rather than hardware factory IRs, SVF-based modal banks, shared delay banks rather than full per-voice waveguides, four MIDI CC proxies rather than eight hardware CV outputs. The complete summation oscillator, hardware communication, editor undo and factory patch data are not implemented. “All” in this pass covers the three feature groups explicitly selected by the user, not a complete hardware engine replacement.
