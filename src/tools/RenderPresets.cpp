#include "engines/Engines.h"
#include "core/Sequencer.h"
#include "plugin/Presets.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
void word(std::ofstream& f,uint32_t v,int n){for(int i=0;i<n;++i)f.put((char)((v>>(i*8))&255));}
void wav(const std::filesystem::path& path,const std::vector<float>& l,const std::vector<float>& r){
    std::ofstream f(path,std::ios::binary);const uint32_t size=(uint32_t)l.size()*4;
    f.write("RIFF",4);word(f,36+size,4);f.write("WAVEfmt ",8);word(f,16,4);word(f,1,2);word(f,2,2);word(f,48000,4);word(f,192000,4);word(f,4,2);word(f,16,2);f.write("data",4);word(f,size,4);
    for(size_t i=0;i<l.size();++i){word(f,(uint16_t)(int16_t)std::lround(std::clamp(l[i],-1.f,1.f)*32767),2);word(f,(uint16_t)(int16_t)std::lround(std::clamp(r[i],-1.f,1.f)*32767),2);}
}
int main(int argc,char** argv){
    if(argc<2 || argc>3){std::cerr<<"usage: triad_render_presets OUTPUT_DIRECTORY [reference|studio|all|smoke]\n";return 2;}
    const std::string bank=argc==3?argv[2]:"reference";
    if(bank!="reference" && bank!="studio" && bank!="all" && bank!="smoke")return 2;
    std::filesystem::path folder(argv[1]);std::filesystem::create_directories(folder);
    std::ofstream report(folder/"audit.csv");report<<"instrument,preset,peak,rms,dc,derivative_rms\n";
    int failures=0;
    for(auto kind:{triad::InstrumentKind::Matriarch,triad::InstrumentKind::PolyBrute,triad::InstrumentKind::MoogOne,triad::InstrumentKind::Continuum}){
        auto e=triad::makeEngine(kind);const auto& d=e->definition();
        for(const auto& preset:triad::factoryPresetsFor(d.id)){
            if(bank=="reference" && !preset.name.endsWith("Ref"))continue;
            if(bank=="studio" && !preset.name.endsWith("[Studio]"))continue;
            if(bank=="smoke" && (!preset.name.endsWith("[Studio]") || preset.category!=(d.id=="continuum"?"Pad":"Arp")))continue;
            std::vector<float> values;for(const auto& spec:d.params)values.push_back(spec.def);
            for(const auto& v:preset.values){int i=d.indexOf(v.first.toStdString());if(i>=0)values[i]=v.second;}
            e->clearPatchCables();e->resetMatrix();e->setMorphAmount(0);
            e->prepare(48000,256);e->setParameters(values.data(),(int)values.size());
            for(const auto& cell:preset.matrix)e->setMatrixCell(cell.source,cell.destination,cell.formula,cell.amount);
            for(const auto& cable:preset.cables)e->setPatchCable(d.patchIndexOf(cable.first.toStdString()),d.patchIndexOf(cable.second.toStdString()),true);
            auto b=values;for(const auto& v:preset.morphB){int i=d.indexOf(v.first.toStdString());if(i>=0)b[i]=v.second;}
            if(!preset.morphB.empty()){e->setMorphSnapshot(b.data(),(int)b.size());e->setMorphAmount(0);}
            constexpr int total=48000*8;std::vector<float> left(total),right(total);
            const bool bass=preset.category=="Bass",pad=preset.category=="Pad" || preset.category=="Strings";
            triad::TransportInfo t;t.bpm=110;t.isPlaying=true;e->setTransport(t);
            const std::string prefix=d.id=="matriarch"?"mat":d.id=="polybrute"?"poly":"one.p1";
            auto parameter=[&](const std::string& suffix,float fallback){int i=d.indexOf(prefix+suffix);return i>=0?values[i]:fallback;};
            const bool arpeggiated=parameter(".arp.enabled",0)>.5f;
            triad::PatternSequencer seq;seq.prepare(48000);seq.setTempo(110);seq.setPlaying(true);
            seq.setMode(arpeggiated?(int)parameter(".arp.mode",0)+1:0);
            seq.setRateIndex((int)parameter(".arp.rate",3));
            seq.setOctaves((int)parameter(".arp.octaves",0)+1);
            std::array<triad::PatternSequencer::Event,triad::PatternSequencer::maxEvents> events;
            for(int pos=0;pos<total;pos+=256){
                const int count=std::min(256,total-pos);
                const int second=pos/48000,previous=(pos-256)/48000;
                if(pos==0 || second!=previous){
                    e->processMidi({triad::MidiEventType::AllNotesOff});seq.allNotesOff();
                    if(second>=6)seq.setMode(0);
                    if(second<6){int note=bass?std::array<int,4>{36,39,41,31}[second%4]:std::array<int,4>{60,63,65,67}[second%4];
                        if(arpeggiated){for(int interval:{0,3,7})seq.noteOn(note+interval,.8f);}
                        else if(pad){for(int interval:{0,4,7})e->processMidi({triad::MidiEventType::NoteOn,0,note-12+interval,0,.75f});}
                        else e->processMidi({triad::MidiEventType::NoteOn,0,note,0,.8f});
                    }
                }
                if(!preset.morphB.empty())e->setMorphAmount(std::clamp(pos/(48000.f*6),0.f,1.f));
                if(kind==triad::InstrumentKind::Continuum){
                    const float phase=pos/(48000.f*6);
                    e->processMidi({triad::MidiEventType::ControlChange,0,0,74,.25f+.5f*std::clamp(phase,0.f,1.f)});
                    e->processMidi({triad::MidiEventType::ChannelPressure,0,0,0,.7f+.2f*std::sin(phase*6.2831853f)});
                }
                if(arpeggiated){
                    const int n=seq.process(count,events.data(),(int)events.size());int cursor=0;
                    for(int j=0;j<n;++j){const auto& event=events[j];
                        if(event.sampleOffset>cursor)e->render(left.data()+pos+cursor,right.data()+pos+cursor,event.sampleOffset-cursor);
                        cursor=event.sampleOffset;
                        e->processMidi({event.noteOn?triad::MidiEventType::NoteOn:triad::MidiEventType::NoteOff,0,event.note,0,event.velocity});
                    }
                    if(cursor<count)e->render(left.data()+pos+cursor,right.data()+pos+cursor,count-cursor);
                }else e->render(left.data()+pos,right.data()+pos,count);
            }
            double energy=0,dc=0,derivative=0;float peak=0,prev=0;
            for(int i=0;i<total;++i){if(!std::isfinite(left[i])||!std::isfinite(right[i])){++failures;break;}float v=(left[i]+right[i])*.5f;energy+=left[i]*left[i]+right[i]*right[i];dc+=v;derivative+=(v-prev)*(v-prev);prev=v;peak=std::max({peak,std::abs(left[i]),std::abs(right[i])});}
            double rms=std::sqrt(energy/(2*total));
            report<<d.id<<",\""<<preset.name<<"\","<<peak<<","<<rms<<","<<dc/total<<","<<std::sqrt(derivative/total)<<"\n";
            if(rms<.00001||peak>=1.0f)++failures;
            std::string name=d.id+"-"+preset.name.toStdString();for(auto& c:name)if(c=='/'||c==':'||c=='\\')c='_';wav(folder/(name+".wav"),left,right);
            if(bank=="smoke")break;
        }
    }
    std::cout<<"Preset audition renders complete; numerical failures: "<<failures<<"\n";return failures?1:0;
}
