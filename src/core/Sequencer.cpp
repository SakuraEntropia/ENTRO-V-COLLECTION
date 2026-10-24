#include "core/Sequencer.h"

#include <algorithm>
#include <cmath>

namespace triad
{

double TapTempo::tap (double milliseconds)
{
    if (! std::isfinite (milliseconds)) return 0.0;
    const auto interval = milliseconds - lastTap_;
    const auto first = lastTap_ < 0.0;
    lastTap_ = milliseconds;
    if (first || interval < 200.0 || interval > 3000.0)
    {
        count_ = next_ = 0;
        return 0.0;
    }
    intervals_[(size_t) next_] = interval;
    next_ = (next_ + 1) % (int) intervals_.size();
    count_ = std::min (count_ + 1, (int) intervals_.size());
    double total = 0.0;
    for (int i = 0; i < count_; ++i) total += intervals_[(size_t) i];
    return 60000.0 * count_ / total;
}

PatternSequencer::PatternSequencer() {
    for (int p = 0; p < maxPatterns; ++p) {
        lengths_[p].store (16);
        for (int s = 0; s < maxSteps; ++s) {
            auto& step = patterns_[p][s];
            for (auto& note : step.notes) note.store (-1);
            step.notes[0].store (48 + (s % 4) * 3);
            step.active.store (s < 16 && s % 2 == 0);
            step.velocity.store (s % 4 == 0 ? 110 : 84);
        }
    }
    sounding_.fill (-1);
}
void PatternSequencer::setLimits (int steps, int notes, int patterns) {
    limitSteps_ = std::clamp (steps, 1, maxSteps);
    limitNotes_ = std::clamp (notes, 1, maxPolyphony);
    limitPatterns_ = std::clamp (patterns, 1, maxPatterns);
    selectPattern (selectedPattern());
}
void PatternSequencer::prepare (double sr) { sampleRate_ = std::isfinite(sr) ? std::clamp(sr,8000.0,384000.0) : 48000.0; reset(); }
void PatternSequencer::reset() {
    heldCount_ = orderIndex_ = patternIndex_ = gateCountdown_ = soundingCount_ = 0;
    sampleAccumulator_ = 0.0; lastPattern_ = -1; previousTie_ = false;
    sounding_.fill (-1); activeNote_.store (-1); activeStep_.store (-1);
}
bool PatternSequencer::valid (int p, int s) const { return p >= 0 && p < limitPatterns_ && s >= 0 && s < limitSteps_; }
void PatternSequencer::selectPattern (int p) { selectedPattern_.store (std::clamp (p,0,limitPatterns_-1)); }
void PatternSequencer::setStepCount (int count) { setPatternLength (selectedPattern(), count); }
int PatternSequencer::stepCount() const { return patternLength(selectedPattern()); }
void PatternSequencer::setPatternLength (int p, int count) { if(p>=0 && p<limitPatterns_) lengths_[p].store(std::clamp(count,1,limitSteps_)); }
int PatternSequencer::patternLength (int p) const { return p>=0 && p<limitPatterns_ ? std::clamp(lengths_[p].load(),1,limitSteps_) : 16; }
void PatternSequencer::setPatternNote (int p,int s,int slot,int note) { if(valid(p,s) && slot>=0 && slot<limitNotes_) patterns_[p][s].notes[slot].store(std::clamp(note,-1,127)); }
int PatternSequencer::patternNote (int p,int s,int slot) const { return valid(p,s) && slot>=0 && slot<limitNotes_ ? patterns_[p][s].notes[slot].load() : -1; }
void PatternSequencer::setPatternActive (int p,int s,bool a) { if(valid(p,s)) patterns_[p][s].active.store(a); }
bool PatternSequencer::patternActive (int p,int s) const { return valid(p,s) && patterns_[p][s].active.load(); }
void PatternSequencer::setPatternVelocity (int p,int s,int v) { if(valid(p,s)) patterns_[p][s].velocity.store(std::clamp(v,1,127)); }
int PatternSequencer::patternVelocity (int p,int s) const { return valid(p,s) ? patterns_[p][s].velocity.load() : 100; }
void PatternSequencer::setPatternTie (int p,int s,bool t) { if(valid(p,s)) patterns_[p][s].tie.store(t); }
bool PatternSequencer::patternTie (int p,int s) const { return valid(p,s) && patterns_[p][s].tie.load(); }
void PatternSequencer::clearPattern (int p) {
    if(p<0 || p>=limitPatterns_) return;
    for(int s=0;s<limitSteps_;++s) { setPatternActive(p,s,false); setPatternTie(p,s,false); for(int n=0;n<limitNotes_;++n) setPatternNote(p,s,n,-1); }
}
void PatternSequencer::setStepActive(int s,bool a) { setPatternActive(selectedPattern(),s,a); }
void PatternSequencer::setStepNote(int s,int n) { setPatternNote(selectedPattern(),s,0,n); }
void PatternSequencer::setStepVelocity(int s,int v) { setPatternVelocity(selectedPattern(),s,v); }
bool PatternSequencer::isStepActive(int s) const { return patternActive(selectedPattern(),s); }
int PatternSequencer::stepNote(int s) const { return patternNote(selectedPattern(),s,0); }
int PatternSequencer::stepVelocity(int s) const { return patternVelocity(selectedPattern(),s); }
void PatternSequencer::noteOn(int note,float velocity) {
    if(note<0 || note>127) return;
    for(int i=0;i<heldCount_;++i) if(heldNotes_[i]==note) return;
    if(heldCount_==maxHeldNotes) { for(int i=1;i<heldCount_;++i) { heldNotes_[i-1]=heldNotes_[i]; heldVelocity_[i-1]=heldVelocity_[i]; } --heldCount_; }
    heldNotes_[heldCount_]=note; heldVelocity_[heldCount_++]=std::clamp(velocity,0.01f,1.0f);
}
void PatternSequencer::noteOff(int note) {
    if(hold_.load()) return;
    for(int i=0;i<heldCount_;++i) if(heldNotes_[i]==note) { for(int j=i+1;j<heldCount_;++j) { heldNotes_[j-1]=heldNotes_[j]; heldVelocity_[j-1]=heldVelocity_[j]; } --heldCount_; break; }
}
void PatternSequencer::allNotesOff() { heldCount_=0; gateCountdown_=1; previousTie_=false; activeStep_.store(-1); }
void PatternSequencer::setHoldActive(bool active) { if(!active) clearHeld(); }
void PatternSequencer::clearHeld() { heldCount_=0; }
int PatternSequencer::pickArpNote() {
    if(!heldCount_) return -1;
    std::array<int,maxHeldNotes> sorted {};
    std::copy_n(heldNotes_.begin(),heldCount_,sorted.begin()); std::sort(sorted.begin(),sorted.begin()+heldCount_);
    int octaves=std::clamp(octaves_.load(),1,4), count=heldCount_*octaves;
    int index=orderIndex_%count;
    switch((Mode)mode_.load()) {
        case Mode::Down: index=count-1-index; break;
        case Mode::UpDown: { int cycle=std::max(1,count*2-2), pos=orderIndex_%cycle; index=pos<count?pos:cycle-pos; break; }
        case Mode::Random: randomState_^=randomState_<<13; randomState_^=randomState_>>17; randomState_^=randomState_<<5; index=(int)(randomState_%count); break;
        default: break;
    }
    ++orderIndex_;
    const auto& notes=(Mode)mode_.load()==Mode::Order?heldNotes_:sorted;
    return std::clamp(notes[index%heldCount_]+12*(index/heldCount_),0,127);
}
int PatternSequencer::process(int samples,Event* events,int capacity) {
    if(samples<=0 || !events || capacity<=0) return 0;
    int written=0;
    auto release=[&](int offset) {
        if(capacity-written<soundingCount_) return false;
        for(int i=0;i<soundingCount_;++i) events[written++]={offset,false,sounding_[i],0.0f};
        soundingCount_=0; activeNote_.store(-1); previousTie_=false; return true;
    };
    Mode mode=(Mode)std::clamp(mode_.load(),0,6);
    if(mode==Mode::Off || !playing_.load()) { release(0); sampleAccumulator_=0.0; gateCountdown_=0; activeStep_.store(-1); return written; }
    double bpm=tempo_.load(); if(!std::isfinite(bpm)) bpm=120.0;
    static constexpr double divisions[]={1,2,3,4,6,8};
    double stepSamples=std::max(4.0,60.0/std::clamp(bpm,20.0,300.0)*sampleRate_/divisions[std::clamp(rateIndex_.load(),0,5)]);
    int pattern=selectedPattern();
    if(pattern!=lastPattern_) { if(!release(0)) return written; patternIndex_=0; sampleAccumulator_=0; lastPattern_=pattern; }
    for(int offset=0;offset<samples;++offset) {
        if(gateCountdown_>0 && --gateCountdown_==0) { if(!release(offset)) { gateCountdown_=1; } }
        if(sampleAccumulator_<=0.0) {
            // Reserve enough room for a whole chord transition, never drop its offs.
            if(capacity-written<soundingCount_+limitNotes_) { sampleAccumulator_-=samples-offset; break; }
            std::array<int,maxPolyphony> next {}; int count=0;
            float velocity=heldCount_?heldVelocity_[0]:0.8f; bool tie=false;
            int step=patternIndex_%patternLength(pattern);
            if(mode==Mode::Step) {
                if(patternActive(pattern,step)) {
                    const int transpose=heldCount_?heldNotes_[0]-60:0;
                    for(int n=0;n<limitNotes_;++n) { int note=patternNote(pattern,step,n); if(note<0) continue; note=std::clamp(note+transpose,0,127); bool duplicate=false; for(int j=0;j<count;++j) duplicate|=next[j]==note; if(!duplicate) next[count++]=note; }
                    velocity=patternVelocity(pattern,step)/127.0f; tie=patternTie(pattern,step);
                }
            } else { int note=pickArpNote(); if(note>=0) next[count++]=note; }
            bool same=previousTie_ && count==soundingCount_ && std::equal(next.begin(),next.begin()+count,sounding_.begin());
            if(!same) { release(offset); for(int n=0;n<count;++n) events[written++]={offset,true,next[n],velocity}; }
            sounding_=next; soundingCount_=count; previousTie_=tie;
            activeNote_.store(count?next[0]:-1); activeStep_.store(step);
            gateCountdown_=tie?0:std::max(1,(int)(stepSamples*std::clamp(gate_.load(),0.05f,1.0f)));
            double swing=std::clamp(swing_.load(),0.0f,0.6f);
            sampleAccumulator_+=stepSamples*(patternIndex_%2==0?1.0+swing:1.0-swing);
            ++patternIndex_;
        }
        sampleAccumulator_-=1.0;
    }
    return written;
}
} // namespace triad
