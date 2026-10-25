#include "plugin/PluginProcessor.h"
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <stdexcept>
static thread_local bool tracking=false;
static thread_local int allocations=0;
void* operator new(std::size_t n){if(tracking)++allocations;if(auto* p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}void operator delete[](void* p)noexcept{std::free(p);}
#define CHECK(x) do{if(!(x)){std::printf("FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(false)
int main(){
    juce::ScopedJuceInitialiser_GUI init;
    triad::TriadAudioProcessor p;
    p.prepareToPlay(48000,512);
    auto set=[&](const char* id,float value){auto* par=p.apvts().getParameter(id);if(!par)throw std::runtime_error(id);par->setValueNotifyingHost(par->convertTo0to1(value));};
    auto layout=p.getBusesLayout();layout.inputBuses.set(0,juce::AudioChannelSet::stereo());CHECK(p.setBusesLayout(layout));
    set("mat.mix.osc1",0);set("mat.mix.osc2",0);set("mat.mix.osc3",0);set("mat.mix.osc4",0);set("mat.mix.noise",0);
    set("mat.master.vcaMode",2);set("mat.delay.mix",0);set("mat.flt.cutoff",10000);set("mat.flt.envAmt",0);set("mat.flt.kbTrack",0);
    juce::AudioBuffer<float> b(2,20000);juce::MidiBuffer midi;midi.ensureSize(8192);
    for(int i=0;i<b.getNumSamples();++i)b.setSample(0,i,.1f*std::sin(i*.04f));b.copyFrom(1,0,b,0,0,b.getNumSamples());
    // Includes oversized blocks and long SysEx, which must not be copied on the callback.
    std::array<juce::uint8,2048> sysex{};midi.addEvent(juce::MidiMessage::createSysExMessage(sysex.data(),(int)sysex.size()),100);
    midi.addEvent(juce::MidiMessage::noteOn(1,60,.8f),35);midi.addEvent(juce::MidiMessage::noteOff(1,60),220);
    allocations=0;tracking=true;p.processBlock(b,midi);tracking=false;
    CHECK(allocations==0);double energy=0;for(int ch=0;ch<2;++ch)for(int i=0;i<b.getNumSamples();++i){CHECK(std::isfinite(b.getSample(ch,i)));energy+=std::abs(b.getSample(ch,i));}CHECK(energy>1);
    layout=p.getBusesLayout();layout.inputBuses.set(0,juce::AudioChannelSet::disabled());CHECK(p.setBusesLayout(layout));
    p.prepareToPlay(48000,512);b.clear();midi.clear();p.processBlock(b,midi);CHECK(b.getMagnitude(0,b.getNumSamples())<1e-7f);
    auto& s=p.sequencer();CHECK(s.patternCount()==12 && s.maximumSteps()==256 && s.polyphony()==4);
    s.selectPattern(11);s.setStepCount(256);s.setPatternNote(11,255,3,87);s.setPatternActive(11,255,true);s.setPatternTie(11,255,true);
    juce::MemoryBlock state;p.getStateInformation(state);s.setPatternNote(11,255,3,21);p.setStateInformation(state.getData(),(int)state.getSize());
    CHECK(s.patternNote(11,255,3)==87 && s.patternTie(11,255) && s.stepCount()==256);
    s.setPatternNote(11,255,3,92);p.getStateInformation(state);s.setPatternNote(11,255,3,21);p.setStateInformation(state.getData(),(int)state.getSize());CHECK(s.patternNote(11,255,3)==92);
    p.keyboardState().noteOn(1,60,.8f);midi.clear();allocations=0;tracking=true;p.processBlock(b,midi);tracking=false;CHECK(allocations==0);
    set("sample.program",1);set("sample.gain",.7f);
    p.getStateInformation(state);set("sample.program",0);p.setStateInformation(state.getData(),(int)state.getSize());
    CHECK(std::abs(p.apvts().getRawParameterValue("sample.program")->load()-1)<.01f);
    b.clear();midi.clear();midi.addEvent(juce::MidiMessage::noteOn(1,60,.9f),0);
    allocations=0;tracking=true;p.processBlock(b,midi);tracking=false;CHECK(allocations==0);
    CHECK(b.getMagnitude(0,b.getNumSamples())>.001f);
    midi.clear();midi.addEvent(juce::MidiMessage::allSoundOff(1),0);b.clear();p.processBlock(b,midi);
    CHECK(b.getMagnitude(0,b.getNumSamples())<1e-7f);
    auto legacy=p.apvts().copyState();
    for(const auto* id:{"sample.program","sample.gain"})legacy.removeChild(legacy.getChildWithProperty("id",id),nullptr);
    juce::MemoryBlock legacyBinary;auto legacyXml=legacy.createXml();
    juce::AudioProcessor::copyXmlToBinary(*legacyXml,legacyBinary);
    p.setStateInformation(legacyBinary.getData(),(int)legacyBinary.getSize());
    CHECK(p.apvts().getRawParameterValue("sample.program")->load()==0);
    std::puts("Host: external bus, large blocks, SysEx, allocation guard, full sequence state passed.");
}
