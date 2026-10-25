#include "engines/SampledEngine.h"
#define TSF_IMPLEMENTATION
#include "../../third_party/tinysoundfont/tsf.h"
#include <array>
#include <filesystem>
#include <mutex>
#include <cstdlib>
#include <fstream>
#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif
namespace triad {
namespace {
std::recursive_mutex& fontMutex(){static auto* mutex=new std::recursive_mutex;return *mutex;}
struct FontDeleter { void operator()(tsf* p)const { std::lock_guard<std::recursive_mutex> guard(fontMutex());if(p)tsf_close(p); } };
using Font=std::unique_ptr<tsf,FontDeleter>;
std::filesystem::path assetFolder() {
    if(const auto* env=std::getenv("ENTRO_SAMPLE_DIR"))return std::filesystem::u8path(env);
    std::filesystem::path module;
#if defined(_WIN32)
    HMODULE handle{};
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      reinterpret_cast<LPCWSTR>(&assetFolder),&handle);
    std::array<wchar_t,32768> name{};GetModuleFileNameW(handle,name.data(),(DWORD)name.size());module=name.data();
#else
    Dl_info info{};if(dladdr(reinterpret_cast<void*>(&assetFolder),&info)&&info.dli_fname)module=info.dli_fname;
#endif
    auto parent=module.parent_path();
    for(int i=0;i<5 && !parent.empty();++i){
        for(const auto& candidate:{parent/"EntroSamples",parent/"Resources"/"EntroSamples"})
            if(std::filesystem::exists(candidate/"UprightKW.sf2"))return candidate;
        parent=parent.parent_path();
    }
    // Source-only tools and tests. Packaged plugins always resolve their bundle resources first.
    return std::filesystem::path(TRIAD_SAMPLE_SOURCE_DIR);
}
std::array<Font,4> loadFonts() {
    std::array<Font,4> result;
    const auto folder=assetFolder();
    const std::array<const char*,4> names{"UprightKW.sf2","ViolinSection.sf2","CelloSection.sf2","SoloViolin.sf2"};
    for(size_t i=0;i<names.size();++i){
        // Use memory loading for Unicode paths, including Windows usernames.
        std::ifstream input(folder/names[i],std::ios::binary|std::ios::ate);
        if(!input)continue;const auto size=input.tellg();if(size<=0 || size>256*1024*1024)continue;
        std::vector<char> data((size_t)size);input.seekg(0);input.read(data.data(),size);
        if(input)result[i].reset(tsf_load_memory(data.data(),(int)data.size()));
    }return result;
}
class SampledEngine final:public SynthEngine {
public:
    explicit SampledEngine(std::unique_ptr<SynthEngine> inner):inner_(std::move(inner)),def_(inner_->definition()) {
        const auto prefix=def_.id=="matriarch"?"mat":def_.id=="polybrute"?"poly":def_.id=="moogone"?"one":"con";
        masterIndex_=def_.indexOf(std::string(prefix)+".master.volume");
        programIndex_=(int)def_.params.size();
        def_.addParam({"sample.program","RECORDED SOURCE","Recorded Instruments","",0,4,0,ParamCurve::Discrete,
                       {"Synthesis","Upright KW","Violin Section","Cello Section","Solo Violin"}});
        gainIndex_=(int)def_.params.size();
        def_.addParam({"sample.gain","SAMPLE LEVEL","Recorded Instruments","",0,1,.65f});
    }
    const ModelDefinition& definition()const override{return def_;}
    void prepare(double rate,int block)override{
        smoothing_=1.0f-std::exp(-1.0f/(float)(rate*.01));
        inner_->prepare(rate,block);scratch_.resize((size_t)std::max(1,block)*2);
        static std::array<Font,4> shared;
        std::lock_guard<std::recursive_mutex> guard(fontMutex());
        if(!shared[0] || !shared[1])shared=loadFonts();
        for(size_t i=0;i<fonts_.size();++i){
            fonts_[i].reset(shared[i]?tsf_copy(shared[i].get()):nullptr);
            if(!fonts_[i])continue;
            tsf_set_output(fonts_[i].get(),TSF_STEREO_INTERLEAVED,(int)rate,std::array<float,4>{-12,6,-6,0}[i]);
            if(!tsf_set_max_voices(fonts_[i].get(),128))throw std::bad_alloc();
            for(int channel=0;channel<16;++channel){
                tsf_channel_set_presetindex(fonts_[i].get(),channel,0);
                tsf_channel_set_pitchrange(fonts_[i].get(),channel,2);
            }
        }
        selectProgram(program_);reset();
    }
    void reset()override{inner_->reset();mute();level_=0;}
    void setParameters(const float* v,int n)override{
        inner_->setParameters(v,std::min(n,programIndex_));
        const int next=n>programIndex_?(int)clampf(std::round(v[programIndex_]),0,4):0;
        if(next!=program_){mute();inner_->reset();program_=next;selectProgram(next);}
        gain_=(n>gainIndex_?clampf(v[gainIndex_],0,1):.65f)*(masterIndex_>=0 && n>masterIndex_?clampf(v[masterIndex_],0,1):1.0f);
    }
    void processMidi(const MidiEvent& e)override{
        if(program_==0){inner_->processMidi(e);return;}
        auto* f=current();if(!f)return;const int c=std::clamp(e.channel,0,15),note=std::clamp(e.note,0,127);
        switch(e.type){
            case MidiEventType::NoteOn:if(e.value>0)tsf_channel_note_on(f,c,note,clampf(e.value,0,1));else tsf_channel_note_off(f,c,note);break;
            case MidiEventType::NoteOff:tsf_channel_note_off(f,c,note);break;
            case MidiEventType::SustainPedal:tsf_channel_set_sustain(f,c,e.value>=.5f);break;
            case MidiEventType::PitchBend:tsf_channel_set_pitchwheel(f,c,(int)((clampf(e.value,-1,1)+1)*8191.5f));break;
            case MidiEventType::ControlChange:tsf_channel_midi_control(f,c,std::clamp(e.controller,0,127),(int)(clampf(e.value,0,1)*127));break;
            case MidiEventType::AllNotesOff:tsf_channel_note_off_all(f,c);break;
            case MidiEventType::AllSoundOff:mute();break;
            default:break;
        }
    }
    void render(float* l,float* r,int n)override{
        if(program_==0){inner_->render(l,r,n);return;}
        auto* f=current();level_=0;
        for(int pos=0;pos<n;){const int count=std::min(n-pos,(int)scratch_.size()/2);
            if(count<=0){std::fill(l+pos,l+n,0);std::fill(r+pos,r+n,0);return;}
            if(f)tsf_render_float(f,scratch_.data(),count,0);else std::fill(scratch_.begin(),scratch_.begin()+count*2,0);
            for(int i=0;i<count;++i){currentGain_+=smoothing_*(gain_-currentGain_);
                l[pos+i]=scratch_[(size_t)i*2]*currentGain_;r[pos+i]=scratch_[(size_t)i*2+1]*currentGain_;
                level_=std::max({level_,std::abs(l[pos+i]),std::abs(r[pos+i])});}pos+=count;
        }
    }
    int activeVoiceCount()const override{return program_==0?inner_->activeVoiceCount():current()?tsf_active_voice_count(current()):0;}
    float outputLevel()const override{return program_==0?inner_->outputLevel():level_;}
    void setMorphSnapshot(const float* p,int n)override{inner_->setMorphSnapshot(p,std::min(n,programIndex_));}
    void setMorphAmount(float v)override{inner_->setMorphAmount(v);}
    void setTransport(const TransportInfo& v)override{inner_->setTransport(v);}
    void setExternalInput(const float* p,int n)noexcept override{inner_->setExternalInput(p,n);}
    bool supportsPatching()const override{return inner_->supportsPatching();}
    void setPatchCable(int a,int b,bool c)override{inner_->setPatchCable(a,b,c);}
    void clearPatchCables()override{inner_->clearPatchCables();}
    int matrixRows()const override{return inner_->matrixRows();}
    int matrixColumns()const override{return inner_->matrixColumns();}
    std::string matrixSourceName(int a)const override{return inner_->matrixSourceName(a);}
    std::string matrixDestinationName(int a)const override{return inner_->matrixDestinationName(a);}
    int matrixFormulaCount()const override{return inner_->matrixFormulaCount();}
    std::string matrixFormulaName(int a)const override{return inner_->matrixFormulaName(a);}
    void setMatrixCell(int a,int b,int c,float v)override{inner_->setMatrixCell(a,b,c,v);}
    int matrixCellFormula(int a,int b)const override{return inner_->matrixCellFormula(a,b);}
    float matrixCellAmount(int a,int b)const override{return inner_->matrixCellAmount(a,b);}
    void resetMatrix()override{inner_->resetMatrix();}
    void setUserFormula(const std::string& v)override{inner_->setUserFormula(v);}
    bool setFormulaText(int a,const std::string& v)override{return inner_->setFormulaText(a,v);}
    std::string formulaText(int a)const override{return inner_->formulaText(a);}
    void setNoteExpression(int a,float b,float c,float d)override{inner_->setNoteExpression(a,b,c,d);}
private:
    tsf* current()const{return program_==0?nullptr:fonts_[(size_t)program_-1].get();}
    void mute(){for(auto& font:fonts_)if(font){
        // tsf_reset frees channel storage; retain preallocated channels on the audio thread.
        for(int i=0;i<font->voiceNum;++i)font->voices[i].playingPreset=-1;
        for(int c=0;c<16;++c){tsf_channel_set_sustain(font.get(),c,0);tsf_channel_set_pitchwheel(font.get(),c,8192);}
    }}
    void selectProgram(int p){if(p>0 && current())for(int c=0;c<16;++c)tsf_channel_set_presetindex(current(),c,0);}
    std::unique_ptr<SynthEngine> inner_;ModelDefinition def_;int programIndex_=0,gainIndex_=0,masterIndex_=-1,program_=0;
    float gain_=.65f,currentGain_=0,smoothing_=.002f,level_=0;std::array<Font,4> fonts_;std::vector<float> scratch_;
};
}
std::unique_ptr<SynthEngine> withSamplePrograms(std::unique_ptr<SynthEngine> engine){return std::make_unique<SampledEngine>(std::move(engine));}
}
