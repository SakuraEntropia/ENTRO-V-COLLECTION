# UI redraw status — 2026-10-22

Branch: `ui/hardware-redraw`. Changes are committed locally. The user explicitly selected local commits only after GitHub's public push was blocked; do not push this branch without new authorization.

## Implemented

- MATRIARCH V: single-row front panel, manual-derived module proportions, staggered patch bay, large cutoff knob, paired long sustain faders, silver knob caps and original coloured sections: yellow ARP, blue oscillators, green filters, purple stereo delay, charcoal utilities/mixer/envelopes/output, blue branding rail. All artwork is drawn in JUCE.
- All **79** Matriarch engine patch points are accessible. The old panel exposed 74; five pre-existing keyboard outputs were missing and are now on the lower rail.
- User follow-up: lower-left **GLIDE / PLAY / HOLD / TAP** controls. PLAY and HOLD use the existing arp parameters. TAP averages four intervals and sets the internal quarter-note tempo, 20–300 BPM; DAW BPM takes priority. Internal tap tempo is transient. GLIDE appends `mat.performance.glide` (0–2 seconds, default zero), preserving the previous 63 IDs and their order. Matriarch now has **64** parameters. First note starts immediately; later note changes slew per oscillator, including paraphonic assignments. Zero glide keeps the existing pitch calculation.
- POLYBRUTE V: three compact synthesis rows, aligned envelope faders, black/white unit style with orange Steiner tab; LCD above the 12×8 source matrix; effects above arpeggiator; lower-left MorphPad with pressure fallback. All 117 parameters accessible.
- ONE V: filled oscillator section, central part console, right filters/envelopes/LFOs/effects and lower modulation matrix. Three parts retained; all 408 parameter IDs retained. The old LFO UI used non-existent `.retrigger` IDs; these now bind the existing `.retrig` parameters.
- CONTINUUM V: light silver-green master/effect parameter rows, six tall narrow inverted trapezoid Delay-LPF controls, full-width light 16×16 matrix, numeric oscillator/filter/bank/delay/shape/envelope blocks below. All 126 parameters accessible. Formula editor, six modes, physical key mapping, red surface and 3D monitor retained. Dragging a Delay-LPF control keeps its original column and sends host change gestures.
- Native-resolution panel snapshots accompany normal editor snapshots (`TRIAD_SNAPSHOT` writes `<path>.panel.png` too).

## Verification

`triad_panel_tests` uses a device-free EditorHost with the actual panel classes and JUCE attachments. It checks every parameter against the engine metadata, host-to-slider and slider/choice-to-host changes, natural-unit numeric text round trips, interactive bounds/overlaps, all 79 patch points and connect/disconnect callbacks, PLAY/HOLD/GLIDE bindings and Delay-LPF endpoints/column capture. It checks each ONE part independently.

`ctest` passed all three targets: engine contracts, factory presets and panel bindings. Engine tests now also check tap averaging/reset and compare actual rendered pitch crossings for immediate, gliding and settled notes. Existing preset tests pass with the additional default-zero glide parameter.

Release build, app screenshots, plugin format validation and installation are recorded in the final verification report accompanying the delivered screenshots.

## Limits

These are reference-based redraws, not measured 1:1 reproductions. The engines still approximate the hardware. Continuum formula/source sets, generated convolution IRs, shared micro delay and CVC MIDI CC proxies retain their existing limitations. Windows has not been run on a real host during this handoff.

## References

Existing manual line drawings and reference images listed in `HANDOFF-GPT.md`. The original-colour Matriarch was additionally checked against the [top-down product photograph](https://www.altomusic.com/cdn/shop/files/d1948-top.jpg) and [manufacturer manual](https://api.moogmusic.com/sites/default/files/2023-03/Matriarch_Manual_012023.pdf). Reference raster art is not used by the UI or packaged as panel artwork.
