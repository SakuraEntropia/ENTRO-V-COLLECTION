# Official catalog research and instrument-specific expansion

Research date: 2026-10-25. Factory catalogs, usable parameter data and independent sound-design studies are distinguished below. Raw vendor installers, firmware and complete original preset payloads are kept outside the repository; no vendor software was installed or executed.

## Official resources inspected

- **Moog One**: the current [official download page](https://www.moogmusic.com/downloads/?product=Moog%20One) links 1.8.2. Its six public Past Preset Packs contain 222 `.m1p` records: CLASSICS 51, TRANSFORMERS 53, STEREO FUTURES 45, KING 26, LISA BELLA DONNA 25, POLYPHONIC DESTINATIONS 22. All names/banks were cataloged; the earlier selected parameter adaptations remain documented separately. This does not assert that every historically released factory or commercial pack is publicly available.
- **Original six-voice PolyBrute**: the [official resources page](https://www.arturia.com/products/hardware-synths/polybrute/resources) provides Connect 3.1.1.1188. Its installer was unpacked without installation. Original-model Factory / Factory 2.0 / Factory 3.0 contain 385 / 64 / 384 serialized records (833 entries, 753 distinct filenames). Counts include any initialization/template entry actually present. PolyBrute 12's two separate 480-entry banks were excluded. Factory libraries are bundled with Connect, as explained by [Arturia](https://support.arturia.com/hc/en-us/articles/10497306470940-PolyBrute-Connect-Manage-your-Sound-library). The catalog does not mean the proprietary modulation/sequence format was completely translated.
- **Matriarch**: all seven current patch resources were retrieved from the public download-page data: Richard Devine, Patching with Intention Vol. 1, Lightbath, Jordan Passmore, Dark Patchbook, Blank Patch and Interconnectivity Patch Sheet 3. The blank sheet is a template rather than a sound bank. Matriarch patches consist of panel/patch-cable settings rather than stored digital presets. Image-only pages were not treated as machine-readable parameter data.
- **Continuum**: [Haken's current release](https://www.hakenaudio.com/editor-and-firmware) is 10.81 (19 October 2026). The linked public Windows archive contains 666 Continuum system-preset MIDI files; Module/Micro/Mini libraries are distinct and excluded. The supplied preset-list PDF is labelled 10.73/10.76; the accompanying current Cookbook and release notes were also inspected. The existing 10.09-name library is not represented as a complete 10.81 DSP recreation.

Complete acquired catalogs are in [official-sounds](official-sounds/). These are snapshots of the public distribution files above, not a promise of access to paid or account-only Sound Store banks. Download hashes are recorded with provenance.

## Design references

PolyBrute's factory Ragtime Piano describes a spectral-analysis approach to piano-like synthesis; Analog Piano, No Human Piano, Cinematic Strings, SoliStrings and Cosmic Cello establish relevant instrument-design directions. Its [official sound-design interviews](https://www.arturia.com/products/hardware-synths/polybrute/sound-design) discuss harmonic blending, dual filtering and complementary A/B timbres. One reference banks include OB STRINGPAD, CHAMBER STRINGS, 1980 VHS STRINGS, OB TO LADDER PAD, PADTASTIC and GRAND SWELL. Haken's Cookbook demonstrates resonator/matrix synthesis; current release notes include Diode Pad Saw, Vioboe and Oboe. The new sounds below are independent designs using these principles, not renamed factory files or claims of audio matches.

## New bank: 256 patches

| Instrument | Atelier acoustic studies | Other signature patches | Saw Atlas pads | Added | Total |
|---|---:|---:|---:|---:|---:|
| MATRIARCH V | 32 | 16 | 0 | 48 | 163 |
| POLYBRUTE V | 32 | 16 | 0 | 48 | 167 |
| ONE V | 32 | 16 | 64 | 112 | 235 |
| CONTINUUM V | 32 | 16 | 0 | 48 | 757 |

Atelier contains four variants of piano, electric piano, solo violin, cello, string section, flute, clarinet and harp. Search `Atelier`, `Signature` or `Saw Atlas` in BROWSE.

- **Matriarch**: four oscillator harmonics, fast amplitude/filter decays for struck strings, shared paraphonic articulation, low-pass stereo filtering and restrained BBD-style ambience. Its shared envelopes and four-note paraphony limit piano independence.
- **PolyBrute**: triangle/Metalizer/FM partials for struck tones; saw/pulse mixtures for bowed/reed tones; separate VCA/filter articulation and velocity. Bowed/ensemble and signature-pad presets provide stored B-side timbres.
- **One**: piano-like fundamental/upper-partial layers have separate amplitude decays. Strings use measured attacks and small vibrato. Saw Atlas uses three polyphonic parts in the 16-voice pool: a stable centre and opposite-panned, detuned wings. All oscillators have saw angle/shape fixed to zero; no pulse blending, oscillator FM or sync. High-cutoff filtering retains saw harmonics; resonant wings add colour while the centre preserves body and mono compatibility. Chorus/reverb are restrained, not the sole source of width. Eight voicing/filter structures × eight cutoff/resonance colours produce 64 pads.
- **Continuum**: five additive/filter routes, odd-harmonic clarinet design, resonator banks and breath/pressure expression. It remains the original EIGEN MATRIX implementation rather than Haken firmware. Instrument studies are synthesized approximations, not piano/string multisamples or measured physical models.

## Checks

The factory suite validates metadata and renders every preset. The signature suite checks 64 pure-saw pad definitions, dry side/mid width and mono body, a measured overtone/fundamental ratio, piano velocity/decay and sustained bowed strings. This is objective signal validation, not a substitute for listening or comparison against hardware/acoustic recordings. No acoustic-fidelity or professional listening certification is claimed.

`triad_render_presets OUTPUT signature` exports all 256 additions at 48 kHz / 16 bit stereo. Existing banks remain available. Install only after tests, full build and validators pass; use the established backup/replacement and Launchpad refresh helpers.

## Verified installed result

All 256 final eight-second renders are finite, non-silent and unclipped, with 256 distinct WAV payloads. Measurements are in [SIGNATURE-AUDIT.csv](SIGNATURE-AUDIT.csv). The complete bank contains 1,322 presets and all entries pass factory metadata/audio tests. All nine CTest suites pass.

One dry-pad checks measured a minimum side/mid RMS ratio of 0.2478 over all 64 pads; representative harmonic checks retain substantial overtones (minimum measured overtone/fundamental energy ratio 3.705 across eight selected structures). Mono body, piano velocity/decay and string sustain checks pass; see [SIGNATURE-QUALITY.txt](SIGNATURE-QUALITY.txt). These readings describe this test setup rather than a universal timbre or listening-quality score.

The four actual installed AU bundles pass fresh auval; all four installed VST3 bundles pass pluginval strictness 5. All 12 bundle signatures and installed executable hashes were verified by the installation helper, previous versions were backed up, and Launchpad registrations were refreshed. The native installer package is 0.2.3. Existing DAW processes/projects were preserved; restart the DAW and reselect a factory patch to apply its updated definition.

Source revision `ffb803e`; cross-platform build: https://github.com/SakuraEntropia/ENTRO-V-COLLECTION/actions/runs/40173487114 (in progress when recorded).
