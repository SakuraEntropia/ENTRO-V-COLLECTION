#include "plugin/Presets.h"
namespace triad {
std::vector<FactoryPreset> analogReferencePresets(const std::string& id) {
    using V=std::vector<std::pair<juce::String,float>>;
    if(id=="matriarch") return {
        // Moog Matriarch Dark Patchbook, printed patch 1 / PDF p4: Night City.
        {"SQ - Night Circuit Ref","Sequence", V{
            {"mat.master.volume",.63f},{"mat.master.paraphony",2},{"mat.performance.glide",.035f},
            {"mat.vco1.octave",0},{"mat.vco2.octave",1},{"mat.vco3.octave",0},{"mat.vco4.octave",1},
            {"mat.vco2.freq",.04f},{"mat.vco4.freq",-.05f},{"mat.mix.osc1",.7f},{"mat.mix.osc2",.65f},
            {"mat.mix.osc3",.23f},{"mat.mix.osc4",.18f},{"mat.mix.noise",.08f},
            {"mat.flt.mode",2},{"mat.flt.cutoff",1600},{"mat.flt.spacing",-.3f},{"mat.flt.res1",.55f},{"mat.flt.res2",.6f},
            {"mat.flt.envAmt",.55f},{"mat.env1.attack",.003f},{"mat.env1.decay",.4f},{"mat.env1.sustain",.12f},{"mat.env1.release",.12f},
            {"mat.env2.attack",.002f},{"mat.env2.decay",.15f},{"mat.env2.sustain",.08f},{"mat.env2.release",.12f},
            {"mat.delay.sync",0},{"mat.delay.time",.37f},{"mat.delay.spacing",.23f},{"mat.delay.mix",.27f},{"mat.delay.feedback",.4f}}, {}, {},
            {{"mat.patch.vco4.out","mat.patch.vco2.fm"}} },
        // Dark Patchbook PDF p11: Loose Sprockets; reinterpret attenuated motion.
        {"LD - Loose Mechanism Ref","Lead",V{
            {"mat.master.volume",.6f},{"mat.master.paraphony",0},{"mat.performance.glide",.06f},
            {"mat.vco1.wave",2},{"mat.vco2.wave",2},{"mat.vco3.wave",2},{"mat.vco4.wave",2},
            {"mat.vco1.octave",1},{"mat.vco2.octave",1},{"mat.vco3.octave",1},{"mat.vco4.octave",2},
            {"mat.vco2.freq",-.06f},{"mat.vco4.freq",3.8f},{"mat.mix.osc1",.65f},{"mat.mix.osc2",.6f},{"mat.mix.osc3",.3f},{"mat.mix.osc4",.17f},
            {"mat.flt.cutoff",780},{"mat.flt.mode",2},{"mat.flt.res1",.42f},{"mat.flt.envAmt",-.25f},
            {"mat.env1.attack",.003f},{"mat.env1.sustain",.8f},{"mat.env1.release",.09f},
            {"mat.lfo2.rate",.72f},{"mat.att1",.04f},{"mat.att3",.15f},
            {"mat.delay.sync",0},{"mat.delay.time",.42f},{"mat.delay.feedback",.34f},{"mat.delay.mix",.2f}}, {}, {},
            {{"mat.patch.lfo2.out","mat.patch.att1.in"},{"mat.patch.att1.out","mat.patch.vco1.pitch"},
             {"mat.patch.fltEnv.out","mat.patch.flt1.cutoff"}} },
        // Dark Patchbook PDF p7 describes different oscillator waveforms and arp.
        {"KY - Four Colour Keys Ref","Keys",V{
            {"mat.master.volume",.61f},{"mat.master.paraphony",2},
            {"mat.vco1.wave",0},{"mat.vco2.wave",1},{"mat.vco3.wave",2},{"mat.vco4.wave",3},
            {"mat.mix.osc1",.7f},{"mat.mix.osc2",.55f},{"mat.mix.osc3",.45f},{"mat.mix.osc4",.4f},
            {"mat.flt.cutoff",3400},{"mat.flt.mode",2},{"mat.flt.spacing",.18f},{"mat.flt.envAmt",.28f},
            {"mat.env1.attack",.005f},{"mat.env1.decay",.8f},{"mat.env1.sustain",.25f},{"mat.env1.release",.55f},
            {"mat.delay.sync",0},{"mat.delay.time",.3f},{"mat.delay.mix",.22f},{"mat.delay.feedback",.28f}} },
        // Dark Patchbook PDF p16: decaying chords and stereo space echoes.
        {"PD - Descending Echo Field Ref","Pad",V{
            {"mat.master.volume",.6f},{"mat.master.paraphony",2},{"mat.performance.glide",.1f},
            {"mat.vco2.freq",-.05f},{"mat.vco4.freq",.055f},{"mat.mix.osc1",.65f},{"mat.mix.osc2",.55f},{"mat.mix.osc3",.4f},{"mat.mix.osc4",.35f},
            {"mat.flt.mode",2},{"mat.flt.cutoff",1800},{"mat.flt.spacing",.3f},{"mat.flt.envAmt",-.28f},
            {"mat.env1.attack",.16f},{"mat.env1.decay",2.8f},{"mat.env1.sustain",.2f},{"mat.env1.release",2.1f},
            {"mat.env2.attack",.32f},{"mat.env2.decay",3.5f},{"mat.env2.sustain",.12f},{"mat.env2.release",3.3f},
            {"mat.lfo2.rate",.13f},{"mat.att3",.07f},{"mat.delay.sync",0},{"mat.delay.time",.58f},{"mat.delay.spacing",.38f},{"mat.delay.feedback",.68f},{"mat.delay.mix",.4f}}, {}, {},
            {{"mat.patch.lfo2.out","mat.patch.att3.in"},{"mat.patch.att3.out","mat.patch.delay.time1Cv"}} }
    };
    if(id!="polybrute") return {};
    // Reconstructed design studies from Arturia's original six-voice PolyBrute
    // sound-design page, not imported factory patch data or audio-match claims.
    return {
        {"PD - A to B Voyage Ref","Pad",V{
            {"poly.master.volume",.55f},{"poly.vcoA.wave",.7f},{"poly.vcoB.wave",.8f},{"poly.vcoB.tune",.045f},
            {"poly.steiner.cutoff",1100},{"poly.ladder.cutoff",1800},{"poly.voice.spread",.8f},
            {"poly.env2.attack",1.2f},{"poly.env2.release",2.8f},{"poly.fx.chorus.mix",.23f},{"poly.fx.reverb.mix",.26f}}, {},
            V{{"poly.vcoA.metalizer",.5f},{"poly.steiner.cutoff",4800},{"poly.ladder.cutoff",6400},
              {"poly.env2.attack",.01f},{"poly.env2.decay",.38f},{"poly.env2.sustain",.25f},
              {"poly.mod.slot1.source",1},{"poly.mod.slot1.dest",9},{"poly.mod.slot1.amount",.15f}} },
        {"KY - Malleable Harp Ref","Keys",V{
            {"poly.master.volume",.6f},{"poly.vcoA.wave",.85f},{"poly.vcoA.metalizer",.18f},{"poly.vcoB.level",.28f},
            {"poly.vcoB.tune",12},{"poly.steiner.cutoff",4200},{"poly.steiner.envAmt",.55f},
            {"poly.env2.attack",.002f},{"poly.env2.decay",.7f},{"poly.env2.sustain",0},{"poly.env2.release",1.2f},
            {"poly.lfo1.rate",.23f},{"poly.mod.slot1.source",1},{"poly.mod.slot1.dest",5},{"poly.mod.slot1.amount",.12f},
            {"poly.fx.delay.mix",.18f},{"poly.fx.reverb.mix",.18f}}, {},
            V{{"poly.vcoA.metalizer",.6f},{"poly.vcoB.fm",.12f},{"poly.steiner.cutoff",6500},{"poly.env2.release",2.1f}} },
        {"SQ - Diminished Night Ref","Sequence",V{
            {"poly.master.volume",.6f},{"poly.vcoA.pulse",.6f},{"poly.vcoB.octave",2},
            {"poly.steiner.cutoff",900},{"poly.steiner.envAmt",.65f},{"poly.ladder.cutoff",2400},
            {"poly.env2.attack",.002f},{"poly.env2.decay",.18f},{"poly.env2.sustain",.15f},{"poly.env2.release",.12f},
            {"poly.arp.enabled",1},{"poly.arp.mode",2},{"poly.arp.rate",3},{"poly.fx.delay.mix",.24f},{"poly.fx.delay.feedback",.3f}} },
        {"PD - First Frost Ref","Pad",V{
            {"poly.master.volume",.53f},{"poly.vcoA.wave",.9f},{"poly.vcoA.pulse",.12f},{"poly.vcoB.wave",.6f},{"poly.vcoB.tune",-.045f},
            {"poly.steiner.cutoff",950},{"poly.ladder.cutoff",1400},{"poly.voice.spread",.95f},
            {"poly.env2.attack",1.8f},{"poly.env2.decay",2.8f},{"poly.env2.sustain",.72f},{"poly.env2.release",4.2f},
            {"poly.fx.chorus.mix",.3f},{"poly.fx.reverb.size",.88f},{"poly.fx.reverb.mix",.32f}}, {},
            V{{"poly.steiner.cutoff",2800},{"poly.ladder.cutoff",4200},{"poly.env2.attack",.4f},{"poly.vcoA.metalizer",.32f}} },
        {"LD - Juvenile Alloy Ref","Lead",V{
            {"poly.master.volume",.58f},{"poly.voice.mode",1},{"poly.vcoA.wave",.7f},{"poly.vcoA.metalizer",.08f},
            {"poly.steiner.cutoff",1800},{"poly.env2.attack",.008f},{"poly.env2.sustain",.8f},{"poly.env2.release",.18f},
            {"poly.fx.delay.mix",.17f}}, {},
            V{{"poly.vcoA.metalizer",.85f},{"poly.vcoB.fm",.2f},{"poly.steiner.cutoff",6500},{"poly.ladder.drive",3.4f}} },
        {"BA - Pulsed Landscape Ref","Bass",V{
            {"poly.master.volume",.6f},{"poly.voice.mode",1},{"poly.vcoA.octave",0},{"poly.vcoB.octave",0},{"poly.vcoB.sub",.4f},
            {"poly.vcoA.pulse",.2f},{"poly.vcoA.metalizer",.3f},{"poly.steiner.cutoff",450},{"poly.steiner.envAmt",.7f},
            {"poly.ladder.cutoff",850},{"poly.env2.attack",.002f},{"poly.env2.decay",.32f},{"poly.env2.sustain",.38f},{"poly.env2.release",.14f},
            {"poly.fx.dist.mix",.18f},{"poly.fx.delay.mix",.06f}} }
    };
}
}
