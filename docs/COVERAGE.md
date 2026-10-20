# Coverage and honest status

Updated 2026-10-20. This table is the source of truth for what exists in code, what automated tests exercise, and what remains unverified.

Legend:

- **Implemented** – the code path exists and is reachable from the panel or MIDI.
- **Tested** – an automated or validator check exercises it (see [VALIDATION.md](VALIDATION.md)).
- **Partial** – the behaviour exists but does not reach the documented hardware scope.
- **Not implemented** – recorded as a gap, not replaced by a decorative control.
- **Calibrated** – compared against hardware recordings/measurements. Nothing is calibrated: no hardware was available.

## Parameter and patch counts

| Instrument | Parameters | Patch points | Factory presets | MIDI handling |
|---|---:|---:|---:|---|
| Matriarch | 63 | 79 | 4 | notes, overlap, sustain, bend, wheel, pressure, all-notes-off |
| PolyBrute | 117 | 0 | 4 | same |
| Moog One | 408 (3 × 133 + 9 master) | 0 | 4 | same, channel → part, split/layer |

All parameters have stable string IDs, natural-unit ranges and group metadata. All are saved in the APVTS tree and exposed to host automation.

## Shared architecture

| Area | Status | Evidence / notes |
|---|---|---|
| Framework-independent core | Implemented, tested | `triad_core` has no JUCE dependency; engine tests link it directly |
| Parameter metadata | Implemented, tested | unique-ID and default-range checks in `EngineTests` |
| Audio engine contract | Implemented, tested | prepare/reset/setParameters/processMidi/render; finite audio at 44.1/48/96 kHz |
| Real-time discipline | Implemented | no allocation in render/MIDI paths; subagent tests instrumented `new` |
| Voice allocation | Implemented | PolyBrute 6 voices; One shared 8-voice pool; Matriarch 1/2/4-note paraphony |
| Sustain / overlap / panic | Implemented, tested | engine contract test and pluginval MIDI tests |
| State persistence | Implemented, tested | APVTS + cables + Morph B + sequencer; pluginval state round-trip |
| Host automation | Implemented, partial | block-rate snapshot; sample-accurate automation is not claimed |
| MIDI event offsets | Implemented | render is split at event sample offsets |
| Keyboard UI | Implemented | Matriarch F1–F5; PolyBrute/One 61 keys C2–C7; wheels send bend/CC1 |
| Presets | Implemented, tested | 4 per instrument; all IDs/ranges validated by `triad_preset_tests` |
| Patch cables | Implemented (Matriarch) | click output → input, right-click clears; saved in state |
| Sequencer/arpeggiator | Partial | one shared 16-step arp/step engine per instance, driven by the host wrapper |

## Matriarch

Reference: 2023-01 manual + firmware 1.3.0 (not re-verified page by page).

| Subsystem | Status | Notes |
|---|---|---|
| Four VCOs | Implemented, tested | octave 16'/8'/4'/2', ±7 st detune (VCO 2–4), triangle/saw/square/narrow pulse, hard sync, linear FM, pulse-width modulation |
| Paraphony | Implemented | 1-note, 2-note, 4-note; one shared filter/envelope path |
| Mixer | Implemented | four oscillator levels + noise, external jack overrides, CP3-style saturation |
| Filters | Implemented, partial | HP/LP series, HP/LP parallel, LP/LP stereo; spacing, resonance, envelope amount, keyboard tracking; HP path is a cascaded 12 dB state-variable approximation rather than the original 24 dB ladder |
| Envelopes | Implemented | two ADSR (2 ms–10 s), sustain, trigger/envelope/end jacks |
| VCA / output | Implemented | Amp Env / Split / Drone, velocity and aftertouch, main volume, paraphony switch |
| Stereo delay | Implemented, partial | BBD-inspired model with time, spacing, feedback, mix, tempo sync, ping-pong; not a component-level BBD model |
| Modulation | Implemented, partial | LFO 1 (six waveforms; staircase approximated by sample-and-hold), pitch/cutoff/PWM amounts, pitch assignment, LFO 2, three bipolar attenuators, two mults |
| Patch points | Implemented, partial | 79 of the documented 90 jacks; one cable per destination; mults and attenuation work; clock/step jacks are generated but the sequencer runs in the host wrapper |
| Patching state | Implemented | atomic connection masks; cables restored from project state |
| Arp / sequencer | Partial | 16-step host sequencer/arp, not the 12 × 256-step hardware sequencer; no pattern editing UI |
| External audio input | Not implemented | rear-panel instrument input has no host bus |
| Tap tempo | Not implemented | tempo follows the host; SYNC/TAP button exists as a parameter |
| Analog calibration | Not implemented | no drift/tolerance calibration against a real unit |

## PolyBrute

Reference: original 6-voice PolyBrute, manual 3.0.1; firmware 3.1.1 differences not fully audited.

| Subsystem | Status | Notes |
|---|---|---|
| Six voices / modes | Implemented, tested | Poly, Mono, Unison 2–4; quietest-then-oldest stealing; glide; spread; analog drift |
| Oscillators | Implemented | VCO A/B octave, tune, continuous saw↔triangle, pulse width/level, Metalizer, sub, soft→hard sync, B→A FM |
| Filters | Implemented, partial | Steiner 12 dB continuous LP→Notch→HP→BP with env/FM/key track; ladder 24 dB with Brute Factor; serial/parallel/split; both self-oscillate; no circuit-level model |
| Envelopes / LFOs | Implemented | three ADSRs with velocity; three per-voice LFOs with sync, retrigger, fade, hardwired pitch/PWM/cutoff |
| Mod matrix | Partial | 8 slots; hardware provides 12 sources / 32 target slots / up to 64 connections |
| Morph A/B | Implemented, partial | all continuous parameters interpolate; discrete choices switch at 0.5; B is captured from the panel (no independent B editing workflow) |
| Effects | Implemented, partial | distortion (4 types), chorus, stereo delay with sync/ping-pong, FDN reverb; not the proprietary algorithms or the full v3 effect list |
| Morphée / ribbon | Not implemented | no XYZ pad or ribbon surface in this build |
| Sequencer / Motion | Not implemented | host-wrapper arp only; no 64-step sequencer or Motion recorder |
| Factory presets | Implemented | 4; not hardware preset data |
| Oversampling | Not implemented | PolyBLEP oscillators and host-rate nonlinear stages; hard sync is a phase reset without BLEP |

## Moog One

Reference: 2019 manual + 1.8.2 release notes; later firmware not audited page by page.

| Subsystem | Status | Notes |
|---|---|---|
| Three parts, shared pool | Implemented, tested | one 8-voice oldest-first pool; per-part enable/range/channel/level/pan; channel routing + split/layer |
| Oscillators | Implemented | three VCOs, waved angle/shape, octave, tune, PW, level; VCO2/3 hard sync; VCO3→VCO1 linear FM |
| Mixer | Implemented, partial | oscillator/noise/ring levels; external input parameter exists but no host side-input bus is wired |
| Filters | Implemented, partial | dual SVF (LP/HP/BP/Notch; 6/12/24 dB approximated) + ladder (6/12/18/24 taps, drive, resonance); series/parallel + mix |
| Envelopes | Implemented | three DAHDSR with velocity and loop options |
| LFOs | Implemented | four per part, seven waves, sync/retrigger/fade; hardwired pitch/PWM/cutoff |
| Mod matrix | Partial | 8 slots per part; the hardware matrix and transforms are deeper |
| Effects | Implemented, partial | per-part distortion/chorus/delay/send + master FDN reverb; **not** an Eventide algorithm or equivalent claim |
| Performance | Implemented, partial | pitch bend, wheels, aftertouch, sustain, glide, drift; no chord memory or unison |
| Arpeggiator / sequencer | Partial | host-wrapper arp uses part 1's arp parameters; no per-part 64-step poly sequencer |
| Factory presets | Implemented | 4; not hardware preset data |
| External audio | Not implemented | engine has a setter; plugin exposes no input bus |

## Platform and delivery

| Item | Status | Notes |
|---|---|---|
| macOS arm64 VST3 | Built, tested | pluginval 1.0.4 strictness 5, including GUI tests |
| macOS AUv2 | Built, tested | `auval` passes for all three |
| macOS standalone | Built, tested | launches; used for screenshots; audio device selection is JUCE's panel |
| Windows x64 | Configured, **not run** | CI workflow written; no Windows runner result yet |
| Universal binary | Supported, **not built** | `-DTRIAD_UNIVERSAL=ON`; only arm64 artifacts inspected |
| Code signing | Ad-hoc only | no Developer ID; no notarisation |
| DAW host test | **Not run** | pluginval and auval only; no Logic/Ableton/Reaper session recorded |
| GitHub push | **Not done** | depends on available credentials |
| Hardware A/B | **Not done** | no measurements or recordings; no “100% emulation” claim |

## Known gaps that matter most

1. No hardware reference recordings, so oscillator/filter/envelope timbre is uncalibrated.
2. Matriarch has 79 of the documented 90 patch points and a reduced sequencer.
3. PolyBrute's Morphée, ribbon, 64-step sequencer and full 12×32 modulation matrix are missing.
4. Moog One's arpeggiator is shared rather than per-part; external audio and chord memory are absent.
5. Windows artifacts and real-DAW loading remain unverified.
6. No VST2, AAX, AUv3 or Linux builds.
