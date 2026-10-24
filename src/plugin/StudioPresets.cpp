#include "plugin/Presets.h"

#include <array>
#include <algorithm>

namespace triad
{
namespace
{
// Eight deliberately different articulation families, each with eight timbral
// variations. No randomness or hardware preset payloads are used.
struct Family
{
    const char* category;
    const char* prefix;
    float cutoff, attack, decay, sustain, release, envelope;
};
constexpr Family families[] {
    {"Bass", "BA", 620, .003f, .32f, .68f, .16f, .42f},
    {"Lead", "LD", 2400, .008f, .5f, .78f, .3f, .22f},
    {"Pad", "PD", 1700, .75f, 2.4f, .76f, 2.6f, .12f},
    {"Keys", "KY", 3900, .003f, .95f, .2f, .7f, .36f},
    {"Pluck", "PL", 2800, .001f, .24f, .015f, .32f, .65f},
    {"Strings", "ST", 2800, .23f, 1.7f, .85f, 1.35f, .12f},
    {"Drone", "DO", 1200, 1.2f, 3.8f, .9f, 3.4f, -.12f},
    {"FX", "FX", 3300, .09f, 1.5f, .48f, 1.7f, -.3f}
};
constexpr const char* names[8][8] {
    {"Velvet Sub", "Rubber Transit", "Copper Pulse", "Broken Acid", "Reese Current", "Roundwood", "Low Voltage", "Octave Furnace"},
    {"Silk Portamento", "Pulse Rider", "Brass Needle", "Sync Razor", "Wide Horizon", "Hollow Reed", "Ribbon Flame", "Fifth Signal"},
    {"Dusk Canopy", "Slow Tides", "Amber Cloud", "Polar Choir", "Orbit Mist", "Velvet Air", "Glass Horizon", "Fifth Aurora"},
    {"Felt Circuit", "Suitcase Spark", "Copper Tines", "Digital Clav", "Chorus Cabinet", "Hollow Celesta", "Prism Keys", "Octave Console"},
    {"Muted Wire", "Pulse Pizzicato", "Copper Harp", "Sync Kalimba", "Stereo Droplets", "Bamboo Spark", "Crystal Pin", "Fifth Koto"},
    {"Soft Ensemble", "Pulse Section", "Copper Bow", "Tremolo Thread", "Wide Chamber", "Hollow Viola", "Glass Rosin", "Fifth Orchestra"},
    {"Undersea Cable", "Tidal Machine", "Copper Planet", "Clockwork Fog", "Reese Nebula", "Hollow Earth", "Glass Current", "Fifth Monolith"},
    {"Submerged Radar", "Pulse Beacon", "Copper Satellites", "Sync Sparks", "Stereo Machinery", "Hollow Transmission", "Glass Collision", "Fifth Launch"}
};
struct Colour { float cutoff, resonance, detune, motion, space; };
constexpr Colour colours[] {
    {.5f, .12f, .025f, .07f, .08f},
    {.8f, .28f, .045f, .16f, .12f},
    {1.2f, .2f, .065f, .24f, .17f},
    {1.8f, .47f, .09f, .38f, .13f},
    {.95f, .18f, .16f, .12f, .28f},
    {1.4f, .34f, .035f, .2f, .21f},
    {2.25f, .23f, .075f, .3f, .32f},
    {1.65f, .16f, .055f, .15f, .24f}
};
using Values = std::vector<std::pair<juce::String, float>>;
void set (Values& values, const juce::String& id, float value)
{
    const auto found = std::find_if (values.begin(), values.end(), [&] (const auto& v) { return v.first == id; });
    if (found == values.end()) values.emplace_back (id, value);
    else found->second = value;
}
void envelope (Values& values, const juce::String& stem, const Family& f, float scale)
{
    const float minimumAttack = stem.startsWith("mat.") || stem.startsWith("poly.") ? .002f : .001f;
    set(values, stem+"attack", std::max(minimumAttack, f.attack * scale));
    set(values, stem+"decay", f.decay * scale);
    set(values, stem+"sustain", f.sustain);
    set(values, stem+"release", f.release * scale);
}

FactoryPreset make (const std::string& id, int family, int variant)
{
    auto f = families[family]; const auto& c = colours[variant];
    const bool rhythm = family == 5 && id != "continuum";
    if (rhythm) f = {"Arp", "AR", 1800, .002f, .18f, .15f, .14f, .55f};
    FactoryPreset p;
    constexpr const char* rhythms[]{"Soft Clock", "Pulse Ladder", "Copper Steps", "Sync Runner", "Wide Cascade", "Hollow Circuit", "Glass Pendulum", "Fifth Relay"};
    p.name = juce::String(f.prefix) + " - " + (rhythm ? rhythms[variant] : names[family][variant]) + " [Studio]";
    p.category = f.category;
    auto& v = p.values;
    const bool bass = family==0, lead=family==1, wide=family==2 || (family==5 && !rhythm) || family==6;
    const float cutoff=f.cutoff*c.cutoff, timeScale=.8f+variant*.055f;
    const int waveform=variant%4;
    if (id=="matriarch")
    {
        set(v,"mat.master.volume",.53f);set(v,"mat.master.paraphony",bass||lead?0:2);
        set(v,"mat.master.vcaMode",0);set(v,"mat.performance.glide",lead?.035f+variant*.008f:.002f);
        for(int osc=1;osc<=4;++osc)
        {
            const auto stem="mat.vco"+juce::String(osc)+".";
            set(v,stem+"octave",bass?0:1);
            set(v,stem+"wave",(waveform+osc-1)%4);
            if(osc>1)set(v,stem+"freq",osc==4 && variant==7?7:c.detune*(osc%2==0?1:-1));
            set(v,"mat.mix.osc"+juce::String(osc),osc==1?.72f:osc==2?.49f:osc==3?.28f:.19f);
        }
        set(v,"mat.mix.noise",family==7?.09f:.005f);set(v,"mat.flt.mode",2);
        set(v,"mat.flt.cutoff",cutoff);set(v,"mat.flt.spacing",wide?.18f+variant*.035f:0);
        set(v,"mat.flt.res1",c.resonance);set(v,"mat.flt.res2",c.resonance*.8f);
        set(v,"mat.flt.envAmt",f.envelope);set(v,"mat.flt.kbTrack",.55f);
        envelope(v,"mat.env1.",f,timeScale);envelope(v,"mat.env2.",f,timeScale*.8f);
        set(v,"mat.delay.sync",0);set(v,"mat.delay.time",.19f+variant*.047f);
        set(v,"mat.delay.mix",bass?.035f:c.space);set(v,"mat.delay.feedback",wide?.48f:.23f);
        if(family>=6)
        {
            set(v,"mat.lfo2.rate",.08f+variant*.11f);set(v,"mat.att3",c.motion*.15f);
            p.cables={{"mat.patch.lfo2.out","mat.patch.att3.in"},{"mat.patch.att3.out","mat.patch.delay.time1Cv"}};
        }
    }
    else if(id=="polybrute")
    {
        set(v,"poly.master.volume",.63f);set(v,"poly.voice.mode",bass||lead?1:0);
        set(v,"poly.voice.spread",wide?.8f:.25f);set(v,"poly.voice.glide",lead?.035f:.002f);
        set(v,"poly.vcoA.octave",bass?0:1);set(v,"poly.vcoB.octave",bass?0:1);
        set(v,"poly.vcoA.wave",variant==0?.88f:variant==5?.6f:.1f+variant*.09f);
        set(v,"poly.vcoB.wave",.25f+variant*.075f);set(v,"poly.vcoA.level",.72f);set(v,"poly.vcoB.level",.5f);
        set(v,"poly.vcoB.tune",variant==7?7:c.detune);set(v,"poly.vcoA.pulse",variant%2?.65f:.1f);
        set(v,"poly.vcoA.pw",.25f+variant*.055f);set(v,"poly.vcoB.sub",bass?.3f:0);
        set(v,"poly.vcoA.metalizer",variant==2?.35f:variant==6?.55f:.04f);
        set(v,"poly.vcoB.sync",variant==3?.55f:0);set(v,"poly.vcoB.fm",family==7?.12f:0);
        set(v,"poly.filters.routing",1);set(v,"poly.filters.balance",.5f);
        set(v,"poly.steiner.cutoff",cutoff);set(v,"poly.ladder.cutoff",cutoff*1.4f);
        set(v,"poly.steiner.resonance",c.resonance);set(v,"poly.ladder.resonance",c.resonance*.7f);
        set(v,"poly.steiner.envAmt",f.envelope);set(v,"poly.ladder.envAmt",f.envelope*.6f);
        set(v,"poly.ladder.drive",1.4f);envelope(v,"poly.env2.",f,timeScale);envelope(v,"poly.env1.",f,timeScale*.75f);
        set(v,"poly.lfo2.rate",.11f+variant*.13f);set(v,"poly.lfo2.pwmAmt",c.motion*.35f);
        set(v,"poly.lfo3.rate",family==7?3.1f+variant:.08f+variant*.09f);
        set(v,"poly.lfo3.cutoffAmt",c.motion*.3f);
        set(v,"poly.fx.chorus.mix",wide?.23f:.06f);set(v,"poly.fx.delay.mix",bass?.025f:c.space*.55f);
        set(v,"poly.fx.reverb.mix",bass?.025f:c.space);set(v,"poly.fx.delay.time",.22f+variant*.035f);
        set(v,"poly.mod.slot1.source",8);set(v,"poly.mod.slot1.dest",9);set(v,"poly.mod.slot1.amount",.23f);
        if(wide || family==7)
            p.morphB={{"poly.steiner.cutoff",cutoff*1.75f},{"poly.vcoA.metalizer",.2f+variant*.08f},
                      {"poly.vcoB.fm",.04f+variant*.025f},{"poly.env2.attack",f.attack*.4f}};
    }
    else if(id=="moogone")
    {
        set(v,"one.master.volume",.48f);set(v,"one.master.voices",1);
        set(v,"one.master.reverbMix",bass?.035f:c.space);set(v,"one.master.reverbSize",.7f);
        for(int part=1;part<=3;++part)
        {
            const auto stem="one.p"+juce::String(part)+".";
            const bool enabled=part==1 || (part==2 && wide) || (part==3 && family==7);
            set(v,stem+"enabled",enabled?1:0);set(v,stem+"level",part==1?.7f:.34f);
            set(v,stem+"pan",wide?(part==1?-.28f:.28f):0);
            set(v,stem+"voice.mode",lead && variant==4?2:bass||lead?1:0);
            set(v,stem+"voice.unisonVoices",3);set(v,stem+"voice.unisonDetune",c.detune);
            for(int osc=1;osc<=3;++osc)
            {
                const auto o=stem+"vco"+juce::String(osc)+".";
                set(v,o+"octave",bass?0:1);set(v,o+"level",osc==1?.7f:osc==2?.48f:.25f);
                set(v,o+"waveAngle",variant==0?.5f:variant==5?.42f:.06f+variant*.04f);
                set(v,o+"shape",variant%2?.6f:.1f);set(v,o+"pw",.24f+variant*.06f);
                set(v,o+"tune",osc==3 && variant==7?7:(osc-1)*c.detune*(part==2?-1:1));
                if(osc>1)set(v,o+"sync",variant==3?1:0);
            }
            set(v,stem+"filters.routing",1);set(v,stem+"filters.mix",variant==5?.2f:.7f);
            set(v,stem+"svf.cutoff",cutoff*(part==2?1.3f:1));set(v,stem+"svf.mode",0);
            set(v,stem+"ladder.cutoff",cutoff);set(v,stem+"ladder.resonance",c.resonance);
            set(v,stem+"svf.resonance",c.resonance*.7f);set(v,stem+"ladder.envAmt",f.envelope);
            set(v,stem+"svf.envAmt",f.envelope*.6f);set(v,stem+"ladder.drive",.08f);
            envelope(v,stem+"env3.",f,timeScale*(part==2?1.3f:1));
            envelope(v,stem+"env1.",f,timeScale*.8f);envelope(v,stem+"env2.",f,timeScale*.8f);
            set(v,stem+"lfo3.rate",.1f+variant*.12f);set(v,stem+"lfo3.cutoffAmt",c.motion*.18f);
            set(v,stem+"fx.chorus.mix",wide?.2f:.04f);set(v,stem+"fx.delay.mix",bass?.025f:c.space*.5f);
            set(v,stem+"fx.delay.time",.21f+variant*.037f);set(v,stem+"fx.reverb.send",.25f);
        }
    }
    else if(id=="continuum")
    {
        set(v,"con.fx.postGain",12);set(v,"con.master.volume",.58f);set(v,"con.master.glide",lead?.035f:.002f);
        set(v,"con.master.reverbMix",bass?.035f:c.space);set(v,"con.master.delayMix",bass?.025f:c.space*.5f);
        set(v,"con.master.yDepth",.65f);set(v,"con.master.zDepth",.75f);set(v,"con.master.noise",0);
        set(v,"con.noise.level",family==7?.025f:.001f);
        for(int osc=1;osc<=5;++osc)
        {
            const auto o="con.osc"+juce::String(osc)+".";
            const float ratios[]={1,1,2,3,4};
            set(v,o+"ratio",osc==1 && bass?.5f:variant==6?1.0f+(osc-1)*.51f:variant==7 && osc>1?ratios[osc-1]*.75f:ratios[osc-1]);
            set(v,o+"wave",variant==0 || family==3 || family==4?0:variant==5?1:2);
            set(v,o+"fine",(osc%2==0?1:-1)*c.detune*45);
            set(v,o+"level",osc==1?.75f:osc==2?.36f:family==3||family==4?.12f:.2f);
            const auto filter="con.flt"+juce::String(osc)+".";
            set(v,filter+"cutoff",std::min(18000.f,cutoff*(1+.2f*osc)));set(v,filter+"res",c.resonance);
            set(v,filter+"env",f.envelope);set(v,filter+"type",0);
            set(v,"con.sg"+juce::String(osc)+".rate",.07f+osc*.09f+variant*.07f);
            set(v,"con.sg"+juce::String(osc)+".level",c.motion*.2f);
            p.matrix.push_back({osc-1,osc-1,1,.6f});
            p.matrix.push_back({osc+5,15,1,.24f});
        }
        envelope(v,"con.env1.",f,timeScale);envelope(v,"con.env2.",f,timeScale*.8f);
        for(int bank=1;bank<=2;++bank)
        {
            auto b="con.bank"+juce::String(bank)+".";
            set(v,b+"freq",220.f*(bank+variant));set(v,b+"mix",family==3||family==4?.27f:.08f);
            set(v,b+"bandwidth",.18f+variant*.065f);set(v,b+"keytrack",1);
        }
        // SG motion and Y/Z expression alter cutoff/timbre, not pitch centre.
        p.matrix.push_back({11,5,3,c.motion*.12f});
        set(v,"con.fx.chorus.mix",wide?.2f:0);set(v,"con.fx.phaser.mix",family==7?.2f:0);
        set(v,"con.fx.delay.mod",family>=6?c.motion*.2f:0);
    }
    if (rhythm)
    {
        const juce::String prefix=id=="matriarch"?"mat":id=="polybrute"?"poly":"one.p1";
        set(v,prefix+".arp.enabled",1);set(v,prefix+".arp.mode",variant%5);
        set(v,prefix+".arp.rate",variant%2==0?3:4);set(v,prefix+".arp.octaves",variant%3+1);
    }
    return p;
}
}

std::vector<FactoryPreset> studioPresets (const std::string& instrumentId)
{
    if(instrumentId!="matriarch" && instrumentId!="polybrute" && instrumentId!="moogone" && instrumentId!="continuum")return {};
    std::vector<FactoryPreset> result;
    result.reserve(64);
    for(int family=0;family<8;++family)
        for(int variant=0;variant<8;++variant)
            result.push_back(make(instrumentId,family,variant));
    return result;
}

std::vector<FactoryPreset> stagePresets (const std::string& id)
{
    if(id!="matriarch" && id!="polybrute" && id!="moogone" && id!="continuum")return {};
    std::vector<FactoryPreset> result;
    constexpr float intervals[]{-.07f,7,12,.11f};
    for(int family=0;family<8;++family)
        for(int colour=0;colour<4;++colour)
        {
            auto p=make(id,family,colour*2+1);
            p.name=p.name.replace("[Studio]","[Stage]");
            auto& v=p.values;
            const auto& f=families[family];
            if(id=="matriarch")
            {
                set(v,"mat.vco2.freq",std::clamp(intervals[colour],-7.f,7.f));
                set(v,"mat.vco3.octave",colour==2?2:1);
                set(v,"mat.vco1.wave",(family+colour)%4);
                set(v,"mat.vco4.wave",(family+colour+2)%4);
                set(v,"mat.flt.cutoff",f.cutoff*(.7f+colour*.4f));
                set(v,"mat.flt.spacing",-.3f+colour*.18f);
                set(v,"mat.env1.decay",f.decay*1.6f);
                set(v,"mat.delay.time",.14f+colour*.095f);
                set(v,"mat.delay.pingPong",colour%2);
            }
            else if(id=="polybrute")
            {
                set(v,"poly.vcoB.tune",intervals[colour]);
                set(v,"poly.vcoA.wave",.8f-colour*.21f);
                set(v,"poly.vcoA.metalizer",.12f+colour*.16f);
                set(v,"poly.vcoB.fm",family==0?.015f:.025f+colour*.025f);
                set(v,"poly.filters.balance",.2f+colour*.2f);
                set(v,"poly.env2.decay",f.decay*1.4f);
                set(v,"poly.mod.slot2.source",9);set(v,"poly.mod.slot2.dest",5);
                set(v,"poly.mod.slot2.amount",.18f);
                if(family==1 && colour==2){set(v,"poly.voice.mode",2);set(v,"poly.voice.unison",1);}
                if(!p.morphB.empty()){
                    set(p.morphB,"poly.vcoB.tune",intervals[colour]+12);
                    set(p.morphB,"poly.vcoA.wave",.15f+colour*.2f);
                }
            }
            else if(id=="moogone")
            {
                for(int part=1;part<=3;++part){
                    const auto stem="one.p"+juce::String(part)+".";
                    set(v,stem+"vco2.tune",intervals[colour]);
                    set(v,stem+"vco1.waveAngle",.45f-colour*.12f);
                    set(v,stem+"svf.mode",colour==3 && family>=6?2:0);
                    set(v,stem+"filters.mix",.22f+colour*.18f);
                    set(v,stem+"env3.decay",(p.category=="Arp"?.18f:f.decay)*1.5f);
                    set(v,stem+"fx.delay.time",.17f+colour*.1f);
                }
                if(family==1 && colour==1){set(v,"one.p1.voice.mode",2);set(v,"one.p1.voice.unisonVoices",4);}
                if(family==3 || family==4){
                    set(v,"one.p2.enabled",1);set(v,"one.p2.level",.2f);
                    set(v,"one.p2.vco1.tune",12);set(v,"one.p2.pan",.32f);
                }
            }
            else
            {
                set(v,"con.osc2.ratio",colour==0?1.5f:colour==1?2.01f:colour==2?2.75f:3.5f);
                set(v,"con.osc3.ratio",colour==0?2.5f:colour==1?3.02f:colour==2?4.25f:5.5f);
                set(v,"con.osc1.wave",(family+colour)%3);
                set(v,"con.env1.decay",f.decay*1.7f);
                set(v,"con.bank1.spread",.15f+colour*.22f);
                set(v,"con.bank1.bandwidth",.15f+colour*.16f);
                set(v,"con.fx.chorus.mix",.08f+colour*.05f);
                set(v,"con.fx.delay.mod",.03f+colour*.04f);
                set(v,"con.sg1.rate",.35f+colour*.43f);
                p.matrix.push_back({12,6,3,.025f+colour*.012f});
            }
            result.push_back(std::move(p));
        }
    return result;
}

}
