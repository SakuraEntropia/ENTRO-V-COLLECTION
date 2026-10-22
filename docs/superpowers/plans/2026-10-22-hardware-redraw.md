# Hardware panel redraw implementation plan

Goal: implement the UI fidelity work authorized in docs/HANDOFF-GPT.md.
Architecture: keep PanelBase attachments and the four engine-facing panel classes; change only presentation and UI diagnostics. All artwork is JUCE vector drawing. C++20 / JUCE 9.0.4.
Spec: docs/HANDOFF-GPT.md, sections 4 and 8.

Constraints: no DSP, parameter ID/layout, preset name, state format, MIDI or MPE changes. Preserve every binding and action; never add unbound decorative controls. References are not shipped panel art. No claim of measured 1:1 reproduction.

- [x] Shared presentation: PanelBase numeric editor using existing SliderAttachment, quiet panel silkscreen, distinct black/white-pointer, black/silver and cream/gold knob families. Keep the host header independently legible.
- [x] MatriarchPanel.cpp: single row, manual-derived relative module widths; two-row top patch bay, aligned control stations, larger filter cutoff and sustain faders. Keep all patch IDs and cable handlers. Check every control fits inside its module.
- [x] PolyBrutePanel.cpp: reduce height and widen synthesis rows; align VCO/filter/envelope/LFO stations, put LCD above source matrix, effects over arp at right, pressure MorphPad at lower left. Preserve all 117 parameter bindings and capture/swap actions.
- [x] OnePanel.cpp: fill left oscillator section and lower LFO section, central part console, right mixer/filter/envelope and lower modulation/effects. Retain async safe part switching and all three prefixes.
- [x] ContinuumPanel.cpp: master rows, effect control panel with six tall narrow inverted trapezoids, full-width light matrix, oscillator/filter/bank/shape/envelope numeric blocks below. Preserve formula editor and every engine-facing interaction.
- [ ] Verification: full release build; ctest; fresh component snapshots plus native-resolution panel snapshots for all four; compare bound IDs against baseline, inspect overlaps/out-of-bounds, inspect each screenshot and repair. Run auval and pluginval for all products. Stage dist and install the reviewed bundles; record actual outcomes and limitations.

Commit independently reviewable analog and Continuum changes. Execute inline in this session.

User follow-ups: original Matriarch colour scheme and functional GLIDE / PLAY / HOLD / TAP. The latter authorizes a minimal default-zero glide parameter/engine addition; see docs/UI-REDRAW-STATUS.md. Keep commits local only.
