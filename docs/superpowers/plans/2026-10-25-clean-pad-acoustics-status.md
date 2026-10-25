# Revision review notes

Implemented design follows explicit user authorization to use recorded samples. The sample adapter appends parameters without changing original IDs or indexes and forwards patch/matrix APIs. Factory Pad cleanup happens after bank assembly so legacy, Studio, Stage and signature pads share the same contract. This alters factory data rather than constraining manual sound design or stored project settings.

The SF2 adapter preallocates channels and voices. TSF reference counts are not atomic; font copies and destruction share a recursive mutex outside render/processMidi. Playback owns independent voices and controllers. Asset discovery uses the containing module on macOS/Windows; Unicode file paths are read with filesystem streams. Every bundle carries matching sample resources; source-tree fallback exists for command-line development tools.

Trade-offs: four source files add about 216 MiB per bundle. Stereo multisamples retain their original pitch/velocity maps, but the current string recordings have finite lengths and no advanced articulation switching. All sampled programs bypass analog controls, explicitly shown in the header. Synthesized flute/clarinet/harp and other studies remain approximate. Old piano/string studies are mapped to the corresponding recorded source; some names are legacy aliases, not new independently recorded instruments.
