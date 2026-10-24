# Coverage and honest status

Updated 2026-10-24. This table is the source of truth for what exists in code, what automated tests exercise, and what remains unverified.

Legend:

- **Implemented** – the code path exists and is reachable from the panel or MIDI.
- **Tested** – an automated or validator check exercises it (see [VALIDATION.md](VALIDATION.md)).
- **Partial** – the behaviour exists but does not reach the documented hardware scope.
- **Not implemented** – recorded as a gap, not replaced by a decorative control.
- **Calibrated** – compared against hardware recordings/measurements. Nothing is calibrated: no hardware was available.

## Parameter and patch counts

| Instrument | Parameters | Patch points | Factory presets | MIDI handling |
|---|---:|---:|---:|---|
| MATRIARCH V | 65 | 79 | 19 | notes, overlap, sustain, bend, wheel, pressure, all-notes-off |
| POLYBRUTE V | 286 | 0 | 23 | same |
| ONE V | 427 | 0 | 27 | notes, overlap, sustain, bend, wheel, pressure, all-notes-off |
| CONTINUUM V | 204 | 0 (16×16 matrix) | 613 | MPE per-note pitch bend / CC74 / pressure; internal 3D surface |

All parameters have stable string IDs, natural-unit ranges and group metadata. All are saved in the APVTS tree and exposed to host automation.

## Shared architecture

| Area | Status | Evidence / notes |
|---|---|---|
| Framework-independent core | Implemented, tested | `triad_core` has no JUCE dependency; engine tests link it directly |
| Parameter metadata | Implemented, tested | unique-ID and default-range checks in `EngineTests` |
| Audio engine contract | Implemented, tested | prepare/reset/setParameters/processMidi/render; finite audio at 44.1/48/96 kHz |
| Real-time discipline | Implemented | no allocation in render/MIDI paths; subagent tests instrumented `new` |
| Voice allocation | Implemented | PolyBrute 6 voices; One selectable shared 8/16-voice pool; Matriarch 1/2/4-note paraphony |
| Global output trim | Implemented | Header `OUT` -60..+12 dB on all four instruments; applies after the engine and is stored in project state, independent of presets |
| Sustain / overlap / panic | Implemented, tested | engine contract test and pluginval MIDI tests |
| State persistence | Implemented, tested | APVTS + cables + Morph B + sequencer; pluginval state round-trip |
| Host automation | Implemented, partial | block-rate snapshot; sample-accurate automation is not claimed |
| MIDI event offsets | Implemented | render is split at event sample offsets |
| Keyboard UI | Implemented | Matriarch F1–F5; PolyBrute/One 61 keys C2–C7; wheels send bend/CC1 |
| Panel layout | Implemented, documented | Matriarch single-row manual order; PolyBrute three-row synth block; One left/centre/right sections (see `docs/LAYOUT.md`) |
| Presets | Implemented, tested | 19 / 23 / 27 / 613 presets; all IDs/ranges validated by `triad_preset_tests` |
| Patch cables | Implemented (Matriarch) | click output → input, right-click clears; saved in state |
| Sequencer/arpeggiator | Partial | banked poly step editor via SEQ: Matriarch 12 × 256 × 4 notes; PolyBrute/One 64 × 6 notes. Ties, velocity, gate, host transport. No hardware recording/ratchets/Motion automation |

## MATRIARCH V

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
| Arp / sequencer | Partial | 12 × 256 steps with up to 4 notes; bank/page editor, ties and velocity. Hardware recording and patch-clock control are incomplete |
| External audio input | Implemented, tested | optional mono/stereo Instrument In bus, mono mix through HOST INPUT mixer control |
| Tap tempo | Not implemented | tempo follows the host; SYNC/TAP button exists as a parameter |
| Analog calibration | Not implemented | no drift/tolerance calibration against a real unit |

## POLYBRUTE V

Reference: original 6-voice PolyBrute, manual 3.0.1; firmware 3.1.1 differences not fully audited.

| Subsystem | Status | Notes |
|---|---|---|
| Six voices / modes | Implemented, tested | Poly, Mono, Unison 2–4; quietest-then-oldest stealing; glide; spread; analog drift |
| Oscillators | Implemented | VCO A/B octave, tune, continuous saw↔triangle, pulse width/level, Metalizer, sub, soft→hard sync, B→A FM |
| Filters | Implemented, partial | Steiner 12 dB continuous LP→Notch→HP→BP with env/FM/key track; ladder 24 dB with Brute Factor; serial/parallel/split; both self-oscillate; no circuit-level model |
| Envelopes / LFOs | Implemented | three ADSRs with velocity; three per-voice LFOs with sync, retrigger, fade, hardwired pitch/PWM/cutoff |
| Mod matrix | Partial | 64 connections, paged eight at a time, 15 non-null software sources and 32 destinations; source arrangement differs from hardware 12 × 32 |
| Morph A/B | Implemented, partial | all continuous parameters interpolate; discrete choices switch at 0.5; B is captured from the panel (no independent B editing workflow) |
| Effects | Implemented, partial | distortion (4 types), chorus, stereo delay with sync/ping-pong, FDN reverb; not the proprietary algorithms or the full v3 effect list |
| Morph performance pad | Implemented, M1 | lower-left 2D pad: X = morph A/B, Y = spread, Z = touch pressure/aftertouch; vertical pressure slider and mouse wheel are the non-touchpad fallback |
| Sequencer / Motion | Partial | 64-step six-note editor with ties/velocity; no Motion recorder |
| Factory presets | Implemented, categorized | 23 presets (including six documented Ref studies) in Bass / Lead / Brass / Pad / Pluck / Keys / Organ / Arp / FX, incl. Reese, Acid, Supersaw and Hoover; not hardware preset data |
| Oversampling | Not implemented | PolyBLEP oscillators and host-rate nonlinear stages; hard sync is a phase reset without BLEP |

## ONE V

Reference: 2019 manual + 1.8.2 release notes; later firmware not audited page by page.

| Subsystem | Status | Notes |
|---|---|---|
| Three parts, shared pool | Implemented, tested | one selectable 8/16-voice oldest-first pool; per-part enable/range/channel/level/pan; channel routing + split/layer |
| Oscillators | Implemented | three VCOs, waved angle/shape, octave, tune, PW, level; VCO2/3 hard sync; VCO3→VCO1 linear FM |
| Mixer | Implemented, partial | oscillator/noise/ring levels; optional mono/stereo host input bus feeds the external mixer |
| Filters | Implemented, partial | dual SVF (LP/HP/BP/Notch; 6/12/24 dB approximated) + ladder (6/12/18/24 taps, drive, resonance); series/parallel + mix |
| Envelopes | Implemented | three DAHDSR with velocity and loop options |
| LFOs | Implemented | four per part, seven waves, sync/retrigger/fade; hardwired pitch/PWM/cutoff |
| Mod matrix | Partial | 8 slots per part; the hardware matrix and transforms are deeper |
| Effects | Implemented, partial | per-part distortion/chorus/delay/send + master FDN reverb; **not** an Eventide algorithm or equivalent claim |
| Performance | Implemented, partial | pitch bend, wheels, aftertouch, sustain, glide, drift; per-part Poly/Mono/Unison, last-note priority, stack size and detune; no chord memory |
| Arpeggiator / sequencer | Partial | host-wrapper arp uses part 1's arp parameters; one shared 64-step six-note sequencer; no independent per-part sequencers |
| Factory presets | Implemented, categorized | 27 presets (including 12 official-parameter Ref adaptations) in Bass / Lead / Brass / Pad / Pluck / Keys / Organ / Arp / Split / FX, incl. Reese, Acid and Supersaw; not hardware preset data |
| External audio | Implemented | optional Instrument In bus, per-part external mixer level |

## Platform and delivery

| Item | Status | Notes |
|---|---|---|
| macOS arm64 VST3 | Built, tested | pluginval 1.0.4 strictness 5, including GUI tests |
| macOS AUv2 | Rebuilt | earlier builds passed auval; this changed revision has not been re-run through auval |
| macOS standalone | Built, tested | launches; used for screenshots; audio device selection is JUCE's panel |
| Windows x64 | Previous revision built in CI | main 309ea79 passed both platform jobs; this revision must be checked separately |
| Universal binary | Supported, **not built** | `-DTRIAD_UNIVERSAL=ON`; only arm64 artifacts inspected |
| Code signing | Ad-hoc only | no Developer ID; no notarisation |
| DAW host test | **Not run** | pluginval and auval only; no Logic/Ableton/Reaper session recorded |
| GitHub remote | Established | SakuraEntropia/ENTRO-V-COLLECTION; current revision validation is recorded separately |
| Hardware A/B | **Not done** | no measurements or recordings; no “100% emulation” claim |

## Known gaps that matter most

1. No hardware reference recordings, so oscillator/filter/envelope timbre is uncalibrated.
2. Matriarch has 79 of the documented 90 patch points and a reduced sequencer.
3. PolyBrute ribbon/Morphée CC sources, 64-step editor and 64 connections exist; Motion, full hardware source layout and calibration remain incomplete.
4. Moog One's arpeggiator/sequencer remains shared across parts; chord memory is absent.
5. This revision needs Windows CI verification; real-DAW loading remains unverified.
6. No VST2, AAX, AUv3 or Linux builds.


## CONTINUUM V / EIGEN MATRIX

Reference: [Haken Continuum / EaganMatrix public documentation](https://www.hakenaudio.com/user-guides), especially the Continuum User Guide §8.4 system-preset categories. This is an original matrix engine, not a copy of EaganMatrix DSP or preset data. The public category codes are `ST`, `WI`, `VO`, `KY`, `CL`, `OT`, `PE`, `PT`, `PR`, `DO`, `MD`, `UT`; our bank uses those codes with original sounds.

| Subsystem | Status | Notes |
|---|---|---|
| Continuous surface | Implemented, M1 | X pitch (fixed note 60 + continuous ±24 st expression, no note retrigger), Y timbre, Z from touchpad pressure (wheel fallback); PITCH SNAP 0..1 blends continuous pitch into semitone-quantised notes; single-pointer UI; MPE input parsed |
| Surface monitor / keyboard | Implemented | 3D display strip above the on-screen keyboard shows circle X/Y position, pressure radius, character line and numeric readout |
| EIGEN MATRIX | Implemented, M1 | 16 sources × 16 destinations; formula + amount per cell; UI click/drag |
| Oscillators | Implemented | five summation-style oscillators, ratio/fine/phase/shape/level/wave |
| Filters | Implemented | five multimode SVF filters, cutoff/resonance/env amount |
| Biquad banks | Implemented, M1 | two 8-band banks; 48-band bank reserved for M2 |
| Delays | Implemented | two delay modules + master delay |
| Shape generators | Implemented | five generators, eight shapes including S&H |
| FDN / reverb | Implemented, approximate | four-line FDN reverb plus chorus, phaser, modulated echo, separate summed/micro delay banks, two generated-IR convolution stages, a six-mode Recirculator selector and six vertical Delay-LPF trapezoid faders (LPF/Offset/PingPong/Feedback/Full/Time); not the original EaganMatrix IR library |
| Panel control coverage | Implemented | All 204 engine parameters have panel controls, including Envelope 1/2, noise colour/level, reverb size and delay time/feedback added in the 2026-10 pass |
| Formula language | Implemented | 12 built-in formulas plus an editable user formula VM (`s/x/y/z`, arithmetic, sin/cos/tan/abs/sqrt/exp/log); compiled to a fixed RPN buffer for the audio thread |
| CVC output | Implemented (MIDI proxy) | Four CVC controls emit MIDI CC 20–23 on channel 1; this is the software equivalent for hosts, not a DC-coupled hardware CV jack |
| CVC | Not implemented | no hardware CVC output |

## Reference-bank update

22 new Ref studies are documented in [PRESET-REFERENCES.md](PRESET-REFERENCES.md). Seven local test suites pass. Standalone audition renders check finite, non-silent, unclipped output; they do not establish hardware fidelity. Ribbon CC9 and Morphée CC114/115/89 are available as modulation sources.
