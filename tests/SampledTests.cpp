#include "engines/Engines.h"
#include "plugin/Presets.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>
using namespace triad;
void check(bool b,const std::string& text){if(!b)throw std::runtime_error(text);}
struct Fixture {
    std::unique_ptr<SynthEngine> e=makeEngine(InstrumentKind::MoogOne);std::vector<float> values,l,r;
    Fixture(){for(const auto& p:e->definition().params)values.push_back(p.def);e->prepare(48000,256);}
    void program(int p){values[(size_t)e->definition().indexOf("sample.program")]=(float)p;e->setParameters(values.data(),(int)values.size());}
    double render(int n){l.resize(n);r.resize(n);for(int p=0;p<n;p+=256)e->render(l.data()+p,r.data()+p,std::min(256,n-p));
        double sum=0;for(int i=0;i<n;++i){check(std::isfinite(l[i]) && std::isfinite(r[i]),"Non-finite sample");check(std::abs(l[i])<1 && std::abs(r[i])<1,"Sample clipping");sum+=l[i]*l[i]+r[i]*r[i];}return std::sqrt(sum/(2*n));}
};
float get(const FactoryPreset& p,const juce::String& id){for(const auto& v:p.values)if(v.first==id)return v.second;throw std::runtime_error("Missing "+id.toStdString());}
int main(){try{
    Fixture f;
    for(int p=1;p<=4;++p){f.program(p);f.e->processMidi({MidiEventType::NoteOn,0,57,0,.8f});
        check(f.render(48000)>.001,"Recorded program missing/silent: "+std::to_string(p));
        f.e->processMidi({MidiEventType::AllSoundOff});
        for(int note:{48,52,55,60,64,67})f.e->processMidi({MidiEventType::NoteOn,0,note,0,1});f.render(48000);
        f.e->processMidi({MidiEventType::AllSoundOff});check(f.render(256)==0,"AllSoundOff must mute immediately");
    }
    f.program(1);f.e->processMidi({MidiEventType::NoteOn,0,60,0,.3f});const auto soft=f.render(24000);f.e->processMidi({MidiEventType::AllSoundOff});
    f.e->processMidi({MidiEventType::NoteOn,0,60,0,.9f});const auto hard=f.render(24000);check(hard>soft*1.5,"Piano velocity response lost");
    f.e->processMidi({MidiEventType::SustainPedal,0,0,0,1});f.e->processMidi({MidiEventType::NoteOff,0,60});check(f.render(48000)>.0005,"Pedal did not sustain piano");
    f.e->processMidi({MidiEventType::SustainPedal,0,0,0,0});f.render(48000*3);check(f.e->activeVoiceCount()==0,"Pedal release left stuck voices");
    f.e->processMidi({MidiEventType::NoteOn,0,60,0,.8f});f.render(1000);f.program(0);check(f.render(256)==0,"Switch to synthesis leaked old samples");
    f.program(1);f.values[(size_t)f.e->definition().indexOf("one.master.volume")]=0;
    f.e->setParameters(f.values.data(),(int)f.values.size());f.e->processMidi({MidiEventType::NoteOn,0,60,0,.9f});
    f.render(48000);check(f.render(256)<1e-7,"Instrument master volume does not mute samples");
    int pads=0;
    for(const auto& id:{"matriarch","polybrute","moogone","continuum"})for(const auto& p:factoryPresetsFor(id)){
        if(p.category!="Pad")continue;++pads;check(p.morphB.empty(),"Pad morph sweeps remain");
        for(const auto& v:p.values){const auto& key=v.first;
            if(key=="poly.steiner.fm" || key.endsWith("envAmt") || key.endsWith("cutoffAmt") || (key.startsWith("con.flt") && key.endsWith(".env")))check(v.second==0,"Time-varying pad filter: "+key.toStdString());
            if(key.endsWith("resonance") || key=="mat.flt.res1" || key=="mat.flt.res2" || (key.startsWith("con.flt") && key.endsWith(".res")))check(v.second<=.061f,"High Pad resonance");
        }
    }
    check(pads>100,"Pad coverage unexpectedly missing");
    std::cout<<"Four recorded programs, velocity, sustain, immediate mute, switching; "<<pads<<" static low-resonance factory pads passed\n";return 0;
}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n';return 1;}}
