#include "engines/PolyBrute.h"
#include <vector>
#include <array>
#include <cmath>
#include <cstdio>
#define CHECK(x) do{if(!(x)){std::printf("FAIL line %d %s\n",__LINE__,#x);return 1;}}while(false)
int main(){
    triad::PolyBruteEngine e; const auto& d=e.definition();std::vector<float> v;
    for(const auto& p:d.params)v.push_back(p.def);
    auto set=[&](const char* id,float n){int i=d.indexOf(id);if(i>=0)v[i]=n;};
    auto run=[&](bool mod){ e.prepare(48000,256);set("poly.voice.analog",0);set("poly.fx.chorus.mix",0);set("poly.fx.delay.mix",0);set("poly.fx.reverb.mix",0);
        set("poly.mod.slot64.source",mod?7:0);set("poly.mod.slot64.dest",1);set("poly.mod.slot64.amount",.5f);
        e.setParameters(v.data(),(int)v.size());e.processMidi({triad::MidiEventType::NoteOn,0,60,0,1});
        std::array<float,256> l,r;std::vector<float> out;
        for(int b=0;b<100;++b){e.render(l.data(),r.data(),256);for(float x:l){if(!std::isfinite(x))return std::vector<float>{};out.push_back(x);}}return out;};
    CHECK(d.find("poly.mod.slot64.amount")!=nullptr);
    CHECK(d.find("poly.mod.slot1.dest")->choices.size()==33);
    auto a=run(false),b=run(true);CHECK(!a.empty() && a.size()==b.size());double diff=0;for(size_t i=0;i<a.size();++i)diff+=std::abs(a[i]-b[i]);CHECK(diff>1);
    for(int slot=1;slot<=64;++slot)CHECK(d.find("poly.mod.slot"+std::to_string(slot)+".source")!=nullptr);
    std::puts("PolyBrute slot 64 audible; 64 connections / 32 destinations passed.");
}
