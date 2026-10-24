#include "core/Sequencer.h"
#include <array>
#include <cstdio>
#include <memory>
#include <set>
#define CHECK(x) do { if(!(x)) { std::printf("FAIL line %d: %s\n",__LINE__,#x); return 1; } } while(false)
int main() {
    auto ptr=std::make_unique<triad::PatternSequencer>(); auto& s=*ptr;
    s.setLimits(256,4,12); s.prepare(48000); s.setTempo(120); s.setRateIndex(3);
    s.clearPattern(11); s.selectPattern(11); s.setStepCount(256);
    s.setPatternActive(11,0,true); s.setPatternActive(11,255,true);
    for(int n=0;n<4;++n) s.setPatternNote(11,0,n,60+n*3);
    s.setPatternNote(11,255,0,84); s.setMode(6); s.setPlaying(true);
    CHECK(s.maximumSteps()==256 && s.polyphony()==4 && s.patternCount()==12);
    std::array<triad::PatternSequencer::Event,512> events;
    auto count=s.process(512,events.data(),512); CHECK(count==4);
    for(int n=0;n<4;++n) CHECK(events[n].noteOn && events[n].sampleOffset==0 && events[n].note==60+n*3);
    bool last=false; std::set<int> sounding;
    for(int n=0;n<count;++n) if(events[n].noteOn) sounding.insert(events[n].note);
    for(int block=0;block<3050;++block) {
        count=s.process(512,events.data(),512);
        for(int n=0;n<count;++n) { if(events[n].noteOn) { sounding.insert(events[n].note); last|=events[n].note==84; } else sounding.erase(events[n].note); }
    }
    CHECK(last); s.setPlaying(false); count=s.process(512,events.data(),512);
    for(int n=0;n<count;++n) if(!events[n].noteOn) sounding.erase(events[n].note);
    CHECK(sounding.empty());
    s.reset(); s.selectPattern(0); s.clearPattern(0); s.setStepCount(2);
    for(int st=0;st<2;++st) {s.setStepActive(st,true);s.setStepNote(st,60);}
    s.setPatternTie(0,0,true); s.setPlaying(true); int ons=0,offs=0;
    for(int n=0;n<13;++n) {count=s.process(512,events.data(),512); for(int i=0;i<count;++i) {ons+=events[i].noteOn;offs+=!events[i].noteOn;}}
    CHECK(ons==1 && offs==0); s.setPlaying(false); CHECK(s.process(512,events.data(),512)==1);
    // Arpeggio swing must alternate; one-octave UpDown does not repeat endpoints.
    s.reset();s.setMode(3);s.setOctaves(1);s.setSwing(0.2f);s.setPlaying(true);
    s.noteOn(60,.8f);s.noteOn(64,.8f);s.noteOn(67,.8f);
    std::array<int,4> expected{60,64,67,64};int k=0;
    for(int block=0;block<50 && k<4;++block) {count=s.process(512,events.data(),512);for(int i=0;i<count && k<4;++i)if(events[i].noteOn)CHECK(events[i].note==expected[k++]);}
    CHECK(k==4); std::puts("Sequencer: 12 banks, 256 steps, chords, tie, swing, stop passed.");
}
