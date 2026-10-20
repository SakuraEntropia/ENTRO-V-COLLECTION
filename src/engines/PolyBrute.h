#pragma once

#include "core/Dsp.h"
#include "core/Engine.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace triad
{

/** PolyBrute: six-voice dual-oscillator morphing analog polysynth.

    Signal path per voice: VCO A (saw/triangle morph with pulse blend and
    Metalizer) + VCO B (same waveform controls, sub sine, hard sync and linear
    FM) + noise -> mixer -> Steiner 12 dB multimode filter and Moog-style
    24 dB ladder (serial / parallel / split) -> stereo pan -> VCA.
    Three envelopes, three per-voice LFOs, an eight-slot modulation matrix,
    and a global distortion / chorus / delay / reverb chain.

    All parameter values are natural units and are indexed in definition()
    order.  setMorphSnapshot() stores a B snapshot; the engine linearly
    interpolates every continuous parameter between A and B while discrete
    parameters switch at morph = 0.5. */
class PolyBruteEngine final : public SynthEngine
{
public:
    PolyBruteEngine();

    const ModelDefinition& definition() const override { return definition_; }

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void setParameters (const float* values, int count) override;
    void setMorphSnapshot (const float* values, int count) override;
    void setMorphAmount (float amount) override;
    void setTransport (const TransportInfo& transport) override;
    void processMidi (const MidiEvent& event) override;
    void render (float* left, float* right, int numSamples) override;

    int activeVoiceCount() const override;
    float outputLevel() const override { return meter_; }

private:
    static constexpr int kNumVoices = 6;
    static constexpr int kMaxHeldNotes = 16;
    static constexpr int kNumModSlots = 8;
    static constexpr int kNumLfos = 3;
    static constexpr int kNumEnvelopes = 3;

    enum ParamId
    {
        PVoiceMode, PVoiceUnison, PVoiceGlide, PVoiceSpread, PVoiceAnalog,
        PVoiceBendRange, PVoiceMorph,

        PVcoAOctave, PVcoATune, PVcoAWave, PVcoAPw, PVcoAPulse, PVcoALevel,
        PVcoAMetalizer,

        PVcoBOctave, PVcoBTune, PVcoBWave, PVcoBPw, PVcoBPulse, PVcoBLevel,
        PVcoBSub, PVcoBSync, PVcoBFm,

        PMixNoise, PMixNoiseColor,

        PSteinerMode, PSteinerCutoff, PSteinerResonance, PSteinerEnvAmt,
        PSteinerLevel, PSteinerKeyTrack, PSteinerFm,

        PLadderCutoff, PLadderResonance, PLadderDrive, PLadderEnvAmt,
        PLadderLevel, PLadderKeyTrack,

        PFiltersRouting, PFiltersBalance,

        PEnv1Attack, PEnv1Decay, PEnv1Sustain, PEnv1Release, PEnv1Velocity,
        PEnv2Attack, PEnv2Decay, PEnv2Sustain, PEnv2Release, PEnv2Velocity,
        PEnv3Attack, PEnv3Decay, PEnv3Sustain, PEnv3Release, PEnv3Velocity,

        PLfo1Rate, PLfo1Wave, PLfo1Sync, PLfo1Retrigger, PLfo1Fade,
        PLfo2Rate, PLfo2Wave, PLfo2Sync, PLfo2Retrigger, PLfo2Fade,
        PLfo3Rate, PLfo3Wave, PLfo3Sync, PLfo3Retrigger, PLfo3Fade,
        PLfo1PitchAmt, PLfo2PwmAmt, PLfo3CutoffAmt,

        PMod1Source, PMod1Dest, PMod1Amount,
        PMod2Source, PMod2Dest, PMod2Amount,
        PMod3Source, PMod3Dest, PMod3Amount,
        PMod4Source, PMod4Dest, PMod4Amount,
        PMod5Source, PMod5Dest, PMod5Amount,
        PMod6Source, PMod6Dest, PMod6Amount,
        PMod7Source, PMod7Dest, PMod7Amount,
        PMod8Source, PMod8Dest, PMod8Amount,

        PFxDistType, PFxDistDrive, PFxDistMix,
        PFxChorusRate, PFxChorusDepth, PFxChorusMix,
        PFxDelayTime, PFxDelayFeedback, PFxDelayMix, PFxDelayPingPong,
        PFxDelaySync,
        PFxReverbSize, PFxReverbDamping, PFxReverbMix,

        PArpEnabled, PArpMode, PArpRate, PArpOctaves, PArpHold, PArpLength,

        PParamCount
    };

    enum ModSource
    {
        SrcNone = 0, SrcLfo1, SrcLfo2, SrcLfo3, SrcEnv1, SrcEnv2, SrcEnv3,
        SrcVelocity, SrcAftertouch, SrcModWheel, SrcKeyTrack, SrcRandom,
        SrcCount
    };

    enum ModDest
    {
        DestNone = 0, DestVcoAPitch, DestVcoBPitch, DestVcoAPw, DestVcoBPw,
        DestMetalizer, DestVcoALevel, DestVcoBLevel, DestNoiseLevel,
        DestSteinerCutoff, DestLadderCutoff, DestSteinerResonance,
        DestLadderResonance, DestFilterALevel, DestFilterBLevel, DestPan,
        DestLfoRates, DestVolume,
        DestCount
    };

    struct Voice
    {
        bool active = false;
        bool held = false;
        bool sustained = false;

        int note = 60;
        float velocity = 0.8f;
        float targetNote = 60.0f;
        float glideNote = 60.0f;
        float detune = 0.0f;
        float pan = 0.0f;
        float randomValue = 0.0f;
        uint64_t age = 0;

        float driftPitchA = 0.0f;
        float driftPitchB = 0.0f;
        float driftCutoff = 0.0f;
        float driftPw = 0.0f;

        float lfoFade[kNumLfos] {};
        float lfoFadeInc[kNumLfos] {};
        float lfoBaseRate[kNumLfos] {};
        float lastSteinerOsc = 0.0f;
        float lastVcoB = 0.0f;

        Rng rng { 0x2545f491u };
        Oscillator oscA;
        Oscillator oscBFree;
        Oscillator oscBSync;
        Oscillator subOsc;
        Lfo lfo[kNumLfos];
        Envelope env[kNumEnvelopes];
        SteinerFilter steiner;
        StateVariableFilter steinerOsc;
        LadderFilter ladder;
        NoiseSource noise;
    };

    struct ModSlot
    {
        int source = 0;
        int dest = 0;
        float amount = 0.0f;
    };

    void buildDefinition();
    void configureVoice (Voice& voice);
    void updateMorph();
    void updateDrift();
    void updateModSlots();
    void clampEffectiveValues();
    void reconcileVoiceMode (int mode, int maxVoices);

    void startVoice (int index, int note, float velocity, bool forceReset,
                     int unisonIndex, int unisonCount);
    void retargetVoice (Voice& voice, int note, float velocity, bool retrigger,
                        int unisonIndex, int unisonCount);
    void releaseVoice (Voice& voice, bool hard);
    int findVoiceToSteal() const;
    int polyphonyForMode() const;

    void noteOn (int note, float velocity);
    void noteOff (int note);
    void allNotesOff (bool hard);
    void allSoundOff();
    void handleSustain (bool down);

    float param (ParamId id) const
    {
        return effective_[(size_t) paramIndex_[(size_t) id]];
    }

    int choice (ParamId id) const
    {
        return std::clamp ((int) std::lround (param (id)), 0, 63);
    }

    static float lfoRateFor (float rateHz, bool tempoSync, double bpm);
    static float oscillatorSample (const Oscillator& osc, float waveMorph,
                                   float pulseWidth, float pulseLevel) noexcept;
    static float metalize (float input, float amount) noexcept;

    ModelDefinition definition_;
    std::array<int, PParamCount> paramIndex_ {};

    std::vector<float> snapshotA_;
    std::vector<float> snapshotB_;
    std::vector<float> effective_;

    float morphAmount_ = 0.0f;
    bool morphExplicit_ = false;

    double sampleRate_ = 48000.0;
    int maxBlockSize_ = 512;

    std::array<Voice, kNumVoices> voices_ {};
    std::array<ModSlot, kNumModSlots> modSlots_ {};

    std::array<int, kMaxHeldNotes> heldNotes_ {};
    std::array<float, kMaxHeldNotes> heldVelocities_ {};
    int heldCount_ = 0;

    std::array<int, kMaxHeldNotes> sustainedNotes_ {};
    int sustainedCount_ = 0;
    bool sustainPedal_ = false;

    float pitchBend_ = 0.0f;   // -1..1
    float modWheel_ = 0.0f;    // 0..1
    float aftertouch_ = 0.0f;  // 0..1
    float lastPlayedNote_ = -1.0f;
    uint64_t ageCounter_ = 0;
    int lastMode_ = -1;
    int lastMaxVoices_ = -1;

    TransportInfo transport_;
    ChorusEffect chorus_;
    StereoDelay delay_;
    ReverbEffect reverb_;
    DCBlocker dcLeft_, dcRight_;
    float meter_ = 0.0f;
};

std::unique_ptr<SynthEngine> makePolyBruteEngine();

} // namespace triad
