# Preset bank and commercial conventions

## What commercial synth plugins do

Checked against the locally installed Serum 2 factory bank and the public manuals of Massive X, u-he Diva and Arturia Pigments.

| Product | Bank size and structure | Browser conventions |
|---|---|---|
| **Serum 2** (Xfer) | 626 factory presets in this installation, split into 27 category folders (Bass 114, Pad 73, Lead 54, Pluck 47, Synth 45, SFX 36, Seq 21, Keyboard 20, Drum 19, Arp 18, Loop 17, Guitar 17, Organ 16, String 13, Soundscape 13, Piano 12, Brass 12, Vox 11, Chord 9, Orchestral 8, Drumkit 8, Mallet 7, Bell 5, Woodwind 4, Template 4, Instrument 3, Hit 3). Names carry a short category prefix such as `BA -`, `LD -`, `PD -`, `PL -`, `KY -`, `OR -`, `BR -`, `STR -`, `ARP -`, `SEQ -`, `FX -`, `BL -`, `CH -`, `EP -` | Folders, Categories and Tags tabs, text search, user ratings, favourites, preset preview, macro controls |
| **Massive X** (Native Instruments) | Factory bank organised by type and character; user presets in a rescanable folder | Browser filters, favourites, search, preset preview, eight macro controls |
| **u-he Diva / Zebra** | Banks by sound designer and category, large third-party soundset ecosystem | Folder/bank browser, category metadata, author metadata |
| **Arturia Pigments** | Large factory library with Type / Genre / Character metadata | Tag-based browsing, macro controls, favourites |

Common practice, in short: a searchable browser rather than a plain dropdown; typed categories; descriptive names often prefixed with a short type code; per-preset metadata; favourites; macros; and a bank large enough to cover many genres. No manufacturer preset data is copied into this project.

## Our bank

The factory bank is original sound design, grouped by the same kinds of categories and using short prefixes.

| Code | Category | MATRIARCH V | POLYBRUTE V | ONE V | CONTINUUM V |
|---|---|---|---:|---:|---:|---:|
| `BA` | Bass | 4 | 4 | 4 | 2 |
| `LD` | Lead | 3 | 4 | 1 | 0 |
| `BR` | Brass | 1 | 1 | 1 | 0 |
| `PD` | Pad | 2 | 3 | 2 | 0 |
| `ST` | Strings | 1 | 0 | 0 | 1 |
| `WI` | Winds | 0 | 0 | 0 | 2 |
| `VO` | Vocal | 0 | 0 | 0 | 2 |
| `KY` | Keyboard | 0 | 1 | 1 | 2 |
| `CL` | Classic | 0 | 0 | 0 | 2 |
| `OT` | Other | 0 | 0 | 0 | 2 |
| `PE` | Percussion | 0 | 0 | 0 | 1 |
| `PT` | Tuned Percussion | 0 | 0 | 0 | 1 |
| `PR` | Processor | 0 | 0 | 0 | 1 |
| `DO` | Drone | 0 | 0 | 0 | 1 |
| `MD` | Midi | 0 | 0 | 0 | 1 |
| `SF` | Surface | 0 | 0 | 0 | 0 |
| `TX` | Texture | 0 | 0 | 0 | 0 |
| `PL` | Pluck | 1 | 1 | 1 | 0 |
| `KEY` | Keys | 0 | 1 | 1 | 0 |
| `OR` | Organ | 1 | 1 | 1 | 0 |
| `ARP` | Arp | 0 | 1 | 1 | 0 |
| `SEQ` | Sequence | 1 | 0 | 0 | 0 |
| `SPL` | Split | 0 | 0 | 1 | 0 |
| `UT` | Utility | 0 | 0 | 0 | 1 |
| `FX` | FX | 1 | 1 | 1 | 0 |
| | **Total** | **15** | **17** | **15** | **19** |

### CONTINUUM V

19 presets using the Continuum's public two-letter system categories:
Strings `ST`, Winds `WI`, Vocal `VO`, Keyboard `KY`, Classic `CL`, Other `OT`,
Percussion `PE`, Tuned Percussion `PT`, Processor `PR`, Drone `DO`, Midi `MD`,
Utility `UT`, plus a small modern Bass group. The matrix cells are part of each
preset; `PR - Broken Matrix`, `OT - Deep Field` and `MD - MPE Surface` carry
custom routing. Names and parameter sets are original; no Haken factory preset
data is included.

Every category has a code used both in the preset name and in the list column, e.g. `BA - Reese Bass`, `LD - Supersaw Lead`, `PD - Warm Analog Pad`.

### Bass highlights

- `BA - Reese Bass` (all three): two or three detuned saws (about ±8–12 cents), low cutoff, light resonance, sub oscillator on PolyBrute, short attack and long sustain.
- `BA - Acid Bass` (all three): mono, short filter envelope, high resonance, drive and distortion.
- `BA - Sub Bass`, `BA - Rubber Bass`, `BA - Metalizer Bass`, `BA - One Bass`, `BA - Sub One`.

### Lead / pad / keys

- `LD - Supersaw Lead` (all three), `LD - Hoover Lead` (PolyBrute), `LD - Sync Lead`, `LD - Metalizer Mono`.
- `PD - Warm Analog Pad`, `PD - Drone Cathedral`, `PD - Morph Pad`, `PD - Big Pad`.
- `PL - Analog Pluck`, `KY - Brute Keys`, `OR - Organ`, `STR - Slow Strings`.

## Browser

The header has a `BROWSE` button that opens a preset browser:

- category filter (All categories / Bass / Lead / ...),
- text search over preset names and categories,
- list with the preset name and its category column,
- detail pane and `LOAD` / `CLOSE` buttons,
- double-click or Return loads; Escape closes,
- selecting a preset does not overwrite the panel until Load is pressed.

The quick preset combo in the header remains available and shows the same category headings.

## Validation

`triad_preset_tests` renders every preset for 0.75 seconds at 48 kHz and checks:

- every ID exists and every value is in range;
- choice indices are valid;
- categories are known, grouped and prefixed;
- at least 12 presets per instrument and at least two bass presets;
- a Reese preset exists;
- audio is finite, audible and below the clipping ceiling.

## Still missing compared with commercial browsers

- No favourites/ratings persistence yet.
- No preset preview audition (hover/arrow auto-play).
- No per-preset macro assignments; our engine parameters are directly automatable instead.
- No third-party bank/author folders; only a factory bank and user `.triadpreset` files.
- Bank size is much smaller than Serum/Pigments; the design prioritises playable, verified sounds over count.
