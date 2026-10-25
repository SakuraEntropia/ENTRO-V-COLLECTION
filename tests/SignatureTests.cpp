#include "engines/Engines.h"
#include "plugin/Presets.h"
#include <cmath>
#include <algorithm>
#include <complex>
#include <iostream>
#include <stdexcept>

using namespace triad;
void require(bool pass,const std::string& message){if(!pass)throw std::runtime_error(message);}
struct Audio {std::vector<float> l,r;};
float value(const FactoryPreset& p,const juce::String& id){for(const auto& v:p.values)if(v.first==id)return v.second;throw std::runtime_error("Missing value "+id.toStdString());}
Audio render(const FactoryPreset& preset,float velocity,bool dry,int seconds=5){
    auto e=makeEngine(InstrumentKind::MoogOne);const auto& def=e->definition();std::vector<float> v;
    for(const auto& spec:def.params)v.push_back(spec.def);
    for(const auto& entry:preset.values)v[def.indexOf(entry.first.toStdString())]=entry.second;
    if(dry)for(size_t i=0;i<def.params.size();++i){const auto& id=def.params[i].id;
        if(id=="one.master.reverbMix" || id.ends_with("fx.chorus.mix") || id.ends_with("fx.delay.mix") || id.ends_with("fx.dist.mix"))v[i]=0;
    }
    e->prepare(48000,256);e->setParameters(v.data(),(int)v.size());e->processMidi({MidiEventType::NoteOn,0,57,0,velocity});
    Audio a;a.l.resize(seconds*48000);a.r.resize(a.l.size());
    for(size_t pos=0;pos<a.l.size();pos+=256){const auto n=std::min<size_t>(256,a.l.size()-pos);e->render(a.l.data()+pos,a.r.data()+pos,(int)n);}
    return a;
}
double rms(const Audio& a,size_t start,size_t n){double sum=0;for(size_t i=start;i<start+n;++i)sum+=(a.l[i]*a.l[i]+a.r[i]*a.r[i])*.5;return std::sqrt(sum/n);}
double harmonic(const Audio& a,int number){
    constexpr int n=24000;const size_t start=a.l.size()-n;double best=0;
    for(double offset:{-3.,-1.5,0.,1.5,3.}){
        const double phase=-2*3.141592653589793*(220*number+offset)/48000;
        const std::complex<double> step(std::cos(phase),std::sin(phase));std::complex<double> osc(1,0),sum(0,0);
        for(int i=0;i<n;++i){const double w=.5-.5*std::cos(2*3.141592653589793*i/(n-1));sum+=(a.l[start+i]+a.r[start+i])*.5*w*osc;osc*=step;}
        best=std::max(best,std::norm(sum));
    }return best;
}
int main(){try{
    const auto patches=signaturePresets("moogone");int count=0;double minimumWidth=100,minimumHarmonics=100;
    const FactoryPreset* piano=nullptr;const FactoryPreset* strings=nullptr;
    for(const auto& p:patches){
        if(p.name=="KY - Concert Hammer [Atelier]")piano=&p;
        if(p.name=="ST - Chamber Violin [Atelier]")strings=&p;
        if(!p.name.endsWith("[Saw Atlas]"))continue;
        for(int part=1;part<=3;++part){auto stem="one.p"+juce::String(part)+".";
            require(value(p,stem+"voice.mode")==0,"Pad must retain polyphony");
            for(int osc=1;osc<=3;++osc){auto s=stem+"vco"+juce::String(osc)+".";
                require(value(p,s+"waveAngle")==0 && value(p,s+"shape")==0,"Pad oscillator lost pure saw character");}
        }
        const auto a=render(p,.75f,true);double mid=0,side=0,peak=0;
        for(size_t i=a.l.size()-48000;i<a.l.size();++i){double m=(a.l[i]+a.r[i])*.5,s=(a.l[i]-a.r[i])*.5;
            require(std::isfinite(m)&&std::isfinite(s),"Non-finite saw pad");mid+=m*m;side+=s*s;peak=std::max({peak,std::abs((double)a.l[i]),std::abs((double)a.r[i])});}
        double width=std::sqrt(side/std::max(mid,1e-20));minimumWidth=std::min(minimumWidth,width);
        require(width>.025,"Dry stereo width collapsed: "+p.name.toStdString());
        require(std::sqrt(mid/48000)>.003,"Pad loses mono body: "+p.name.toStdString());require(peak<.98,"Pad clipping");
        if(count%8==3){double overtones=0;for(int h=2;h<=6;++h)overtones+=harmonic(a,h);
            double ratio=overtones/std::max(1e-20,harmonic(a,1));minimumHarmonics=std::min(minimumHarmonics,ratio);
            require(ratio>.035,"Pad filtered into a near-sine/triangle: "+p.name.toStdString());}
        ++count;
    }
    require(count==64,"Expected 64 saw pads");require(piano && strings,"Missing acoustic profiles");
    auto soft=render(*piano,.3f,true),hard=render(*piano,.9f,true),bow=render(*strings,.8f,true);
    require(rms(hard,0,48000)>rms(soft,0,48000)*1.4,"Piano does not respond to velocity");
    require(rms(hard,hard.l.size()-48000,48000)<rms(hard,0,48000)*.5,"Piano amplitude fails to decay");
    require(rms(bow,bow.l.size()-48000,48000)>.003,"Bowed string loses sustain");
    std::cout<<"64 pure-saw pads: minimum dry side/mid="<<minimumWidth<<", minimum overtone/fundamental="<<minimumHarmonics<<"; piano velocity/decay and string sustain passed.\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
