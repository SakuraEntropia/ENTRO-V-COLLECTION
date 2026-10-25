#include "plugin/Presets.h"
#include <algorithm>
#include <array>

namespace triad {
namespace {
using Values=std::vector<std::pair<juce::String,float>>;
void put(Values& v,const juce::String& id,float x) {
    auto i=std::find_if(v.begin(),v.end(),[&](const auto& p){return p.first==id;});
    if(i==v.end())v.emplace_back(id,x);else i->second=x;
}
void env(Values& v,const juce::String& stem,float a,float d,float s,float r) {
    put(v,stem+"attack",a);put(v,stem+"decay",d);put(v,stem+"sustain",s);put(v,stem+"release",r);
}
struct Instrument {
    const char* category;const char* prefix;
    float attack,decay,sustain,release,cutoff,noise;
};
constexpr Instrument instruments[]{
    {"Keyboard","KY",.002f,3.1f,0,.3f,5400,.002f},
    {"Keys","KY",.002f,2.2f,0,.5f,4200,0},
    {"Strings","ST",.12f,1.4f,.85f,.48f,3600,.006f},
    {"Strings","ST",.18f,1.8f,.88f,.65f,2200,.008f},
    {"Strings","ST",.55f,2.6f,.86f,1.6f,4600,.004f},
    {"Winds","WI",.035f,.5f,.8f,.18f,5400,.03f},
    {"Winds","WI",.02f,.6f,.82f,.22f,3200,.006f},
    {"Pluck","PL",.002f,1.35f,0,.65f,6200,.002f}
};
constexpr const char* acousticNames[8][4]{
    {"Felt Grand","Concert Hammer","Studio Upright","Bright Stage Piano"},
    {"Warm Tines","Glass Suitcase","Bark Reed Keys","Copper Electric"},
    {"Solo Bow","Rosin Violin","Chamber Violin","Silk Cantabile"},
    {"Dark Cello","Wood Cello","Chamber Cello","Cello Harmonics"},
    {"Small String Section","Wide Orchestral Strings","Muted Chamber Strings","Cinema Bow Section"},
    {"Concert Flute","Breathy Alto Flute","Wooden Flute","Air Harmonics"},
    {"Chalumeau Clarinet","Solo Clarinet","Reed Chamber","Warm Bass Clarinet"},
    {"Concert Harp","Muted Nylon","Resonant Koto","Bright Wire Harp"}
};
FactoryPreset acoustic(const std::string& id,int type,int colour,const FactoryPreset& base) {
    auto p=base;p.name=juce::String(instruments[type].prefix)+" - "+acousticNames[type][colour]+" [Atelier]";
    p.category=instruments[type].category;p.morphB.clear();p.cables.clear();
    auto& v=p.values;const auto& f=instruments[type];
    const bool struck=type<2 || type==7,strings=type>=2 && type<=4,flute=type==5,clarinet=type==6;
    const float a=std::max(.002f,f.attack*(.7f+colour*.25f)),d=f.decay*(.75f+colour*.18f),r=f.release*(.8f+colour*.2f);
    const float cutoff=f.cutoff*(.72f+colour*.19f),detune=type==4?.07f:strings?.018f:.006f;
    if(id=="matriarch") {
        put(v,"mat.master.volume",.6f);put(v,"mat.master.paraphony",2);put(v,"mat.performance.glide",0);
        put(v,"mat.arp.enabled",0);put(v,"mat.output.velocity",struck?.85f:.45f);
        for(int osc=1;osc<=4;++osc){const auto s="mat.vco"+juce::String(osc)+".";
            put(v,s+"wave",strings?1:clarinet?2:0);put(v,s+"octave",struck?(osc==1?1:osc==4?3:2):1);
            if(osc>1)put(v,s+"freq",struck && osc==3?7:detune*(osc%2?1:-1));
            put(v,"mat.mix.osc"+juce::String(osc),osc==1?.72f:osc==2?(flute?.06f:.28f):osc==3?.12f:.06f);
        }
        put(v,"mat.mix.noise",f.noise);put(v,"mat.flt.mode",2);put(v,"mat.flt.cutoff",cutoff);
        put(v,"mat.flt.spacing",type==4?.18f:0);put(v,"mat.flt.res1",.08f);put(v,"mat.flt.res2",.1f);
        put(v,"mat.flt.envAmt",struck?.42f:.12f);put(v,"mat.flt.kbTrack",.65f);
        env(v,"mat.env1.",a,d,f.sustain,r);env(v,"mat.env2.",.002f,struck?.32f:.7f,struck?0:.35f,.2f);
        put(v,"mat.lfo.wave",0);put(v,"mat.lfo.rate",5.1f+colour*.17f);put(v,"mat.lfo.pitchAmt",strings?.002f:0);
        put(v,"mat.delay.mix",type==4?.14f:.065f);put(v,"mat.delay.feedback",.18f);put(v,"mat.delay.time",.29f+colour*.025f);
    } else if(id=="polybrute") {
        put(v,"poly.master.volume",.66f);put(v,"poly.voice.mode",0);put(v,"poly.voice.glide",.002f);
        put(v,"poly.arp.enabled",0);put(v,"poly.voice.spread",type==4?.8f:.18f);
        put(v,"poly.vcoA.octave",1);put(v,"poly.vcoB.octave",struck?2:1);
        put(v,"poly.vcoA.wave",strings?.05f:1);put(v,"poly.vcoB.wave",strings?.12f:1);
        put(v,"poly.vcoB.tune",struck?7:detune);put(v,"poly.vcoA.pulse",clarinet?.52f:0);put(v,"poly.vcoB.pulse",0);
        put(v,"poly.vcoA.level",.75f);put(v,"poly.vcoB.level",flute?.07f:struck?.2f:.47f);
        put(v,"poly.vcoA.metalizer",type==1?.18f+colour*.05f:struck?.06f:0);
        put(v,"poly.vcoB.fm",type==1?.035f:0);put(v,"poly.vcoB.sync",0);put(v,"poly.vcoB.sub",0);
        put(v,"poly.filters.routing",1);put(v,"poly.filters.balance",.45f);put(v,"poly.steiner.mode",0);
        put(v,"poly.steiner.cutoff",cutoff);put(v,"poly.ladder.cutoff",cutoff*1.3f);
        put(v,"poly.steiner.resonance",.1f);put(v,"poly.ladder.resonance",.1f);
        put(v,"poly.steiner.envAmt",struck?.45f:.12f);put(v,"poly.ladder.envAmt",struck?.25f:.1f);
        env(v,"poly.env2.",a,d,f.sustain,r);env(v,"poly.env1.",.002f,struck?.28f:.7f,struck?0:.35f,.22f);
        put(v,"poly.env2.velocity",struck?.9f:.45f);put(v,"poly.lfo1.rate",5.2f);put(v,"poly.lfo1.pitchAmt",strings?.0025f:0);
        put(v,"poly.lfo2.pwmAmt",0);put(v,"poly.lfo3.cutoffAmt",0);
        put(v,"poly.fx.chorus.mix",type==4?.18f:type==1?.1f:0);put(v,"poly.fx.delay.mix",.035f);put(v,"poly.fx.reverb.mix",.16f);
        if(strings)p.morphB={{"poly.steiner.cutoff",cutoff*1.5f},{"poly.ladder.cutoff",cutoff*1.6f},{"poly.env2.attack",a*.6f}};
    } else if(id=="moogone") {
        put(v,"one.master.volume",.56f);put(v,"one.master.voices",1);put(v,"one.master.reverbMix",.14f);
        for(int part=1;part<=3;++part){const auto s="one.p"+juce::String(part)+".";
            const bool layer=part==1 || (part==2 && (struck || type==4));
            put(v,s+"enabled",layer?1:0);put(v,s+"level",part==1?.7f:type==4?.3f:.13f);
            put(v,s+"pan",type==4?(part==1?-.3f:.5f):part==2?.15f:0);
            put(v,s+"voice.mode",0);put(v,s+"arp.enabled",0);
            for(int osc=1;osc<=3;++osc){const auto o=s+"vco"+juce::String(osc)+".";
                put(v,o+"waveAngle",strings?0:.5f);put(v,o+"shape",clarinet?.58f:0);put(v,o+"pw",.5f);
                put(v,o+"octave",struck?(osc==1?1:2):1);put(v,o+"tune",struck && osc==3?7:detune*(osc-1)*(part==2?-1:1));
                put(v,o+"level",osc==1?.7f:osc==2?(flute?.06f:.22f):.1f);
                if(osc>1)put(v,o+"sync",0);
            }
            put(v,s+"vco3.fm",type==1?.055f+colour*.015f:0);put(v,s+"mix.noise",f.noise*.5f);
            put(v,s+"mix.ring",0);put(v,s+"filters.routing",1);put(v,s+"filters.mix",.25f);
            put(v,s+"svf.mode",0);put(v,s+"svf.cutoff",cutoff);put(v,s+"ladder.cutoff",cutoff*1.2f);
            put(v,s+"svf.resonance",.08f);put(v,s+"ladder.resonance",.1f);
            put(v,s+"svf.envAmt",struck?.4f:.12f);put(v,s+"ladder.envAmt",struck?.3f:.1f);
            put(v,s+"svf.keyTrack",.7f);put(v,s+"ladder.keyTrack",.6f);
            env(v,s+"env3.",a,part==2 && struck?.12f:d,part==2 && struck?0:f.sustain,r);
            put(v,s+"env3.velocity",struck?.9f:.45f);
            env(v,s+"env1.",.001f,struck?.26f:.8f,struck?0:.35f,.2f);env(v,s+"env2.",.001f,struck?.32f:.9f,struck?0:.3f,.23f);
            put(v,s+"lfo1.rate",5.1f+colour*.2f);put(v,s+"lfo1.pitchAmt",strings?.0025f:0);
            put(v,s+"lfo3.cutoffAmt",0);put(v,s+"fx.chorus.mix",type==4?.14f:type==1?.1f:0);
            put(v,s+"fx.delay.mix",.025f);put(v,s+"fx.reverb.send",.2f);
        }
    } else {
        put(v,"con.master.volume",.6f);put(v,"con.fx.postGain",12);put(v,"con.master.glide",.001f);
        put(v,"con.master.noise",0);put(v,"con.noise.level",f.noise);put(v,"con.master.reverbMix",.15f);put(v,"con.master.delayMix",.025f);
        for(int osc=1;osc<=5;++osc){auto o="con.osc"+juce::String(osc)+".";auto q="con.flt"+juce::String(osc)+".";
            put(v,o+"wave",strings?2:clarinet && osc==1?3:0);
            put(v,o+"ratio",strings?1:clarinet?float(std::min(7,osc*2-1)):float(osc));
            put(v,o+"fine",strings?(osc%2?1:-1)*detune*100:colour*.12f*(osc-1));
            put(v,o+"level",osc==1?.72f:flute?.08f/osc:struck?.34f/osc:.3f/osc);
            put(v,q+"cutoff",std::min(17000.f,cutoff*(1+.13f*osc)));put(v,q+"type",0);
            put(v,q+"env",struck?.45f:.12f);put(v,q+"res",.08f);
        }
        env(v,"con.env1.",a,d,f.sustain,r);env(v,"con.env2.",.001f,struck?.25f:.7f,struck?0:.35f,.24f);
        put(v,"con.bank1.freq",type==3?220:type==6?900:440);put(v,"con.bank2.freq",type==3?660:type==6?2700:1320);
        put(v,"con.bank1.mix",struck?.2f:.12f);put(v,"con.bank2.mix",struck?.12f:.08f);
        put(v,"con.bank1.keytrack",1);put(v,"con.bank2.keytrack",1);
        put(v,"con.fx.chorus.mix",type==4?.16f:type==1?.08f:0);
    }
    return p;
}

FactoryPreset sawPad(int family,int colour) {
    FactoryPreset p;p.category="Pad";
    constexpr const char* families[]{"Ladder Vista","Open Horizon","Resonant Cathedral","Silk Continent","Prismatic Saw","Fifth Skyline","Moving Copper","Mono Anchor"};
    constexpr const char* colours[]{"Amber","Pearl","Cobalt","Rose","Silver","Indigo","Gold","Crimson"};
    p.name=juce::String("PD - ")+families[family]+" "+colours[colour]+" [Saw Atlas]";
    auto& v=p.values;put(v,"one.master.volume",.46f);put(v,"one.master.voices",1);put(v,"one.master.drift",.025f);
    put(v,"one.master.reverbMix",.12f+family*.008f);put(v,"one.master.reverbSize",.72f);
    constexpr float cutoffs[]{2400,3200,4200,5600,7200,9000,11200,14000};
    constexpr float resonance[]{.22f,.3f,.39f,.48f,.57f,.65f,.73f,.81f};
    for(int part=1;part<=3;++part){const auto s="one.p"+juce::String(part)+".";
        put(v,s+"enabled",1);put(v,s+"midiChannel",0);put(v,s+"level",part==1?.54f:.31f);
        put(v,s+"pan",part==1?0:part==2?-.94f:.94f);put(v,s+"voice.mode",0);put(v,s+"arp.enabled",0);
        for(int osc=1;osc<=3;++osc){auto o=s+"vco"+juce::String(osc)+".";
            put(v,o+"waveAngle",0);put(v,o+"shape",0);put(v,o+"octave",1);put(v,o+"level",osc==1?.65f:osc==2?.4f:.25f);
            float tune=part==1?(osc==1?0:osc==2?.025f:-.025f):part==2?-.055f-osc*.012f:.055f+osc*.013f;
            if(family==5 && part==3 && osc==3)tune+=7;
            put(v,o+"tune",tune);put(v,o+"beatHz",0);if(osc>1)put(v,o+"sync",0);
        }
        put(v,s+"vco3.fm",0);put(v,s+"mix.noise",0);put(v,s+"mix.ring",0);put(v,s+"mix.ext",0);
        put(v,s+"filters.routing",1);put(v,s+"filters.mix",part==1?.2f:family==1?.35f:.82f);
        put(v,s+"svf.mode",family==2 && part>1?2:0);put(v,s+"svf.slope",family==1?0:1);
        put(v,s+"svf.cutoff",part==1?std::max(6500.f,cutoffs[colour]):cutoffs[colour]*(part==2?.9f:1.1f));
        put(v,s+"ladder.cutoff",cutoffs[colour]);put(v,s+"ladder.slope",family%4);
        put(v,s+"svf.resonance",part==1?.07f:resonance[colour]*.7f);
        put(v,s+"ladder.resonance",part==1?.12f:resonance[colour]);put(v,s+"ladder.drive",.035f);
        put(v,s+"svf.envAmt",.08f);put(v,s+"ladder.envAmt",family==6?.24f:.12f);
        put(v,s+"svf.keyTrack",.45f);put(v,s+"ladder.keyTrack",.4f);
        const float attack=.45f+family*.19f+colour*.07f;
        env(v,s+"env3.",attack+(part==3?.17f:0),2.6f,.84f,2.8f+colour*.26f);
        put(v,s+"env3.velocity",.45f);env(v,s+"env1.",attack*.7f,3,.65f,2.6f);env(v,s+"env2.",attack,3.5f,.6f,3);
        put(v,s+"lfo1.pitchAmt",0);put(v,s+"lfo2.pwmAmt",0);put(v,s+"lfo3.rate",.045f+family*.012f+part*.009f);
        put(v,s+"lfo3.cutoffAmt",part==1?.012f:.035f+colour*.005f);
        put(v,s+"lfo4.rate",.06f+part*.017f);put(v,s+"mod.slot1.source",10);put(v,s+"mod.slot1.dest",11);put(v,s+"mod.slot1.amount",.16f);
        put(v,s+"mod.slot2.source",9);put(v,s+"mod.slot2.dest",9);put(v,s+"mod.slot2.amount",.1f);
        put(v,s+"fx.dist.mix",0);put(v,s+"fx.chorus.mix",family==3?.09f:.035f);
        put(v,s+"fx.chorus.depth",.16f);put(v,s+"fx.delay.mix",.035f);put(v,s+"fx.delay.time",.37f+part*.021f);
        put(v,s+"fx.delay.feedback",.23f);put(v,s+"fx.reverb.send",.16f);
    }
    return p;
}
}
std::vector<FactoryPreset> signaturePresets(const std::string& id) {
    auto bases=studioPresets(id);if(bases.size()!=64)return {};
    std::vector<FactoryPreset> result;
    for(int type=0;type<8;++type)for(int c=0;c<4;++c)result.push_back(acoustic(id,type,c,bases[24+c*2]));
    if(id=="moogone") {
        for(int f=0;f<8;++f)for(int c=0;c<8;++c)result.push_back(sawPad(f,c));
        for(int i=0;i<16;++i){auto p=bases[i<8?i:i<12?8+(i-8)*2:40+(i-12)*2];p.name=p.name.replace("[Studio]","[Signature]");
            for(int part=1;part<=3;++part){auto s="one.p"+juce::String(part)+".";
                put(p.values,s+"vco2.tune",i%4==0?-.075f:i%4==1?.06f:i%4==2?7:12);
                put(p.values,s+"filters.mix",.22f+(i%4)*.18f);put(p.values,s+"ladder.drive",.16f+(i%4)*.045f);
                put(p.values,s+"env3.velocity",.68f);put(p.values,s+"fx.delay.mix",i<8?.025f:.12f);
            }result.push_back(std::move(p));}
    } else {
        constexpr const char* titles[]{"Amber Architecture","Living Filaments","Copper Panorama","Resonant Silk"};
        for(int f=0;f<4;++f)for(int c=0;c<4;++c){auto p=bases[16+c*2];p.name=juce::String("PD - ")+titles[f]+" "+juce::String(c+1)+" [Signature]";
            auto& v=p.values;
            if(id=="matriarch") {
                put(v,"mat.flt.cutoff",1900+f*550+c*320);put(v,"mat.flt.res1",.18f+c*.12f);put(v,"mat.flt.res2",.23f+c*.1f);
                put(v,"mat.vco2.freq",.035f+f*.025f);put(v,"mat.vco4.freq",-.04f-c*.018f);
                put(v,"mat.flt.spacing",.2f+c*.13f);put(v,"mat.delay.spacing",.15f+f*.09f);put(v,"mat.delay.mix",.18f);
            } else if(id=="polybrute") {
                put(v,"poly.vcoA.wave",.08f+f*.17f);put(v,"poly.steiner.cutoff",2200+c*900);put(v,"poly.ladder.cutoff",3200+f*700);
                put(v,"poly.steiner.resonance",.2f+c*.13f);put(v,"poly.ladder.resonance",.2f+f*.1f);put(v,"poly.voice.spread",.9f);
                p.morphB={{"poly.vcoA.wave",.8f},{"poly.vcoA.metalizer",.22f+c*.1f},{"poly.steiner.cutoff",6500+f*1400},{"poly.ladder.cutoff",7200+c*1000}};
            } else {
                for(int osc=1;osc<=5;++osc){auto s="con.osc"+juce::String(osc)+".";
                    put(v,s+"wave",f==2?0:2);put(v,s+"ratio",f==2?float(osc):1);
                    put(v,s+"fine",(osc%2?1:-1)*(3+f+c*.8f));put(v,"con.flt"+juce::String(osc)+".cutoff",2600+c*800+osc*350);
                    put(v,"con.flt"+juce::String(osc)+".res",.15f+f*.1f);
                }put(v,"con.bank1.mix",.12f+c*.04f);put(v,"con.fx.chorus.mix",.15f);put(v,"con.master.reverbMix",.2f);
            }result.push_back(std::move(p));
        }
    }
    return result;
}
}
