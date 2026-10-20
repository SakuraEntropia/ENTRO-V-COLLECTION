# Analog Triad approved design

Approved by the user on 2026-10-20: develop the three-instrument suite in the accompanying Chinese research proposal, macOS and Windows, then push to GitHub. Ordinary implementation proceeds without further design approval.

## Product and boundaries

Three independent, self-contained audio instruments built from one C++20 repository. Each has a separate synthesis engine and hardware-oriented panel. Deliver VST3 on macOS and Windows x64, AUv2 on macOS, and standalone applications. JUCE is pinned to 9.0.4 (3fb5cbc0340758596dac39f7c8de47d562b4896e).

Full documented functional coverage is the final target. Releases must list actual coverage and gaps. Independent DSP implementations are not evidence of circuit or proprietary-effects equivalence. No claim of complete hardware emulation without corresponding tests and measurements.

## Modules

- `src/core`: framework-independent parameter definitions, engine interface, bounded DSP primitives and timing.
- `src/engines`: Matriarch, PolyBrute and One definitions and independent implementations. No JUCE types.
- `src/plugin`: JUCE processor, MIDI dispatch, atomic parameter snapshots, state persistence, presets and host buses.
- `src/ui`: vector-drawn physical-panel editor, parameter attachments, screen controls and virtual patching.
- `tests`: deterministic offline audio and engine behavior tests; plugin state/format validation.
- `scripts`, `.github/workflows`: reproducible builds, packaging and platform verification.

## Stable interface

`ModelDefinition` owns static parameter and patch-port metadata. `SynthEngine` accepts parameter and patch snapshots, transport and MIDI events, and returns one stereo sample. Preparation allocates resources; rendering does not allocate, access files, or wait for locks. A plugin instance creates one engine, selected at compile time.

Parameters have stable string IDs, units, explicit ranges/defaults, and grouping. All parameters are persisted; patch assignments are saved with versioned state. Input event offsets are respected. The initial host wrapper applies parameter snapshots per block; do not claim sample-accurate host automation.

## Instrument requirements

Matriarch: four oscillators and 1/2/4-note paraphony; normalized modular routing; stereo filters, envelopes, modulation, attenuators, delay and sequencing. PolyBrute: six voices, two oscillators/sub/noise, Metalizer, Steiner/Ladder, A/B parameter morph, expression and matrix routing, sequencing and effects. One: shared 8/16 voice pool across three parts, three oscillators, SVF/Ladder, three envelopes/four LFOs per voice, modulation, performance, sequencers and effects. Firmware baselines and hidden settings are tracked in `docs/COVERAGE.md`.

## Acceptance

Every implemented control must change the corresponding engine behavior and round-trip through saved state. Instruments produce finite audio at 44.1/48/96 kHz, handle note overlap, sustain, pitch bend, all-notes-off, state reset, sample-rate changes and varied block sizes. Matriarch allocation is paraphonic, PolyBrute morph changes synthesis parameters, and One parts share the selected pool. Host compatibility requires format validators and actual DAW records. Only platforms actually built and tested are marked verified.

## Delivery and source control

Develop in `outputs/analog-triad`; generated build files and downloaded dependencies remain outside the repository. Stage installable user-facing builds under `outputs`. Use the existing authorized GitHub account SakuraEntropia and a private `analog-triad` repository if available. No unrelated repository changes. Report remote or signing limitations accurately; never replace unfinished requirements with completion claims.
