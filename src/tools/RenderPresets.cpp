#include "engines/Engines.h"
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
    if(argc!=2){std::cerr<<"usage: triad_render_presets OUTPUT_DIRECTORY\n";return 2;}
    std::filesystem::path folder(argv[1]);std::filesystem::create_directories(folder);
    std::ofstream report(folder/"audit.csv");report<<"instrument,preset,peak,rms,dc,derivative_rms\n";
    int failures=0;
    for(auto kind:{triad::InstrumentKind::Matriarch,triad::InstrumentKind::PolyBrute,triad::InstrumentKind::MoogOne}){
        auto e=triad::makeEngine(kind);const auto& d=e->definition();
        for(const auto& preset:triad::factoryPresetsFor(d.id)){
            if(!preset.name.endsWith("Ref"))continue;
            std::vector<float> values;for(const auto& spec:d.params)values.push_back(spec.def);
            for(const auto& v:preset.values){int i=d.indexOf(v.first.toStdString());if(i>=0)values[i]=v.second;}
            e->prepare(48000,256);e->setParameters(values.data(),(int)values.size());
            for(const auto& cable:preset.cables)e->setPatchCable(d.patchIndexOf(cable.first.toStdString()),d.patchIndexOf(cable.second.toStdString()),true);
            auto b=values;for(const auto& v:preset.morphB){int i=d.indexOf(v.first.toStdString());if(i>=0)b[i]=v.second;}
            if(!preset.morphB.empty()){e->setMorphSnapshot(b.data(),(int)b.size());e->setMorphAmount(0);}
            constexpr int total=48000*8;std::vector<float> left(total),right(total);
            const bool bass=preset.category=="Bass",pad=preset.category=="Pad";
            triad::TransportInfo t;t.bpm=110;t.isPlaying=true;e->setTransport(t);
            for(int pos=0;pos<total;pos+=256){
                const int count=std::min(256,total-pos);
                const int second=pos/48000,previous=(pos-256)/48000;
                if(pos==0 || second!=previous){
                    e->processMidi({triad::MidiEventType::AllNotesOff});
                    if(second<6){int note=bass?std::array<int,4>{36,39,41,31}[second%4]:std::array<int,4>{60,63,65,67}[second%4];
                        if(pad){for(int interval:{0,4,7})e->processMidi({triad::MidiEventType::NoteOn,0,note-12+interval,0,.75f});}
                        else e->processMidi({triad::MidiEventType::NoteOn,0,note,0,.8f});
                    }
                }
                if(!preset.morphB.empty())e->setMorphAmount(std::clamp(pos/(48000.f*6),0.f,1.f));
                e->render(left.data()+pos,right.data()+pos,count);
            }
            double energy=0,dc=0,derivative=0;float peak=0,prev=0;
            for(int i=0;i<total;++i){if(!std::isfinite(left[i])||!std::isfinite(right[i])){++failures;break;}float v=(left[i]+right[i])*.5f;energy+=left[i]*left[i]+right[i]*right[i];dc+=v;derivative+=(v-prev)*(v-prev);prev=v;peak=std::max({peak,std::abs(left[i]),std::abs(right[i])});}
            double rms=std::sqrt(energy/(2*total));
            report<<d.id<<",\""<<preset.name<<"\","<<peak<<","<<rms<<","<<dc/total<<","<<std::sqrt(derivative/total)<<"\n";
            if(rms<.00001||peak>=1.0f)++failures;
            std::string name=d.id+"-"+preset.name.toStdString();for(auto& c:name)if(c=='/'||c==':'||c=='\\')c='_';wav(folder/(name+".wav"),left,right);
        }
    }
    std::cout<<"Preset audition renders complete; numerical failures: "<<failures<<"\n";return failures?1:0;
}
