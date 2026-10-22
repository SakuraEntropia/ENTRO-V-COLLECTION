# Hardware detail pass

Baseline: 99e35af, including the other agent's Continuum expression fixes, 128 parameters and 613 presets. Preserve those changes. Keep commits local.

- [x] ONE: three small oscillator OLEDs using the same Oscillator sampling and angle/shape/pulse blending as the engine; automatically follow selected part and host automation. Do not draw invented traces.
- [x] PolyBrute: move the existing functional MorphPad into the keyboard-left performance recess; move pitch/mod wheels into the upper-left control panel; preserve capture/swap, X/Y/Z, mouse-pressure fallback and wheel callbacks. Redraw the tapered timber touch block and cabinet silhouette from the straight-on photo/manual.
- [x] Cabinet details: instrument-specific rails, corners, knob skirts and keyboard/performance proportions; no new unbound controls or panel-photo assets.
- [x] Continuum: document a source-linked feature gap audit against current official engine/editor guides, separating existing approximations from genuinely absent modules. Do not silently treat researching further functionality as a new engine rewrite.
- [x] Run existing meaningful panel/engine/preset tests, capture fresh full editor and native panel images, build the four standalone/AU/VST3 products and record actual validation state.

User follow-up: corrected oscillator hexagons from the detailed product photo, appended real default-zero beat-frequency controls, and expanded the three selected Continuum feature groups. See UI-REDRAW-STATUS.md for limitations.
