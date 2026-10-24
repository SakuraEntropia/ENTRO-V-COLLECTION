#pragma once

#include "core/Dsp.h"
#include "core/Engine.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace triad
{

/** Moog One style polysynth.

    Three independent parts share one selectable pool of eight or sixteen voices. A voice is allocated
    globally, oldest first, and renders with the parameter snapshot of the part
    that triggered it. Each part owns three VCOs (sync and linear FM), a noise /
    ring / external-input mixer, a dual state-variable filter plus a ladder filter,
    three DAHDSR envelopes, four LFOs, an eight-slot modulation matrix and a
    distortion / chorus / delay / reverb-send effect chain. A master reverb and
    volume stage complete the signal path.

    MIDI event channels are zero-based (0 = MIDI channel 1), matching the host
    adapter. The per-part midiChannel menu stores the same zero-based index while
    showing 1..16. Parts that share a MIDI channel are split or layered by their
    key ranges; notes on a channel no enabled part listens to play the selected
    part.

    @note The arp.* parameters are metadata for the host arpeggiator; the engine
    does not run an arpeggiator. External audio is injected through
    setExternalInput() (the mixer's EXT level multiplies it; with no buffer it is
    silence). Pitch-bend range is global. Mono and unison use last-note priority;
    unison consumes the same shared voice pool as poly mode. Chord memory is not
    implemented. The 6 dB SVF slope blends the 12 dB stage with its input. */
class OneEngine final : public SynthEngine
{
public:
    static constexpr int kPartCount = 3;
    static constexpr int kMaxVoices = 16;
    static constexpr int kLfoCount = 4;
    static constexpr int kEnvCount = 3;
    static constexpr int kModSlotCount = 8;
    static constexpr int kMaxHeldNotes = 128 * 16;

    OneEngine();

    const ModelDefinition& definition() const override { return definition_; }

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void setParameters (const float* values, int count) override;
    void setTransport (const TransportInfo& transport) override;
    void processMidi (const MidiEvent& event) override;
    void render (float* left, float* right, int numSamples) override;

    int activeVoiceCount() const override { return activeVoices_; }
    float outputLevel() const override { return meter_; }

    /** Optional mono external input for the mixer's EXT level.
        The pointer is read during the next render() call only; a null pointer
        (the default) is silence. */
    void setExternalInput (const float* samples, int numSamples) noexcept override
    {
        externalInput_ = samples;
        externalInputSize_ = samples != nullptr ? numSamples : 0;
    }

private:
    // -----------------------------------------------------------------------
    // Parameter identity. Values are stored in definition() order; these enums
    // address the cached index table rather than re-searching strings.
    // -----------------------------------------------------------------------
    enum PartParam
    {
        PEnabled, PKeyboardLow, PKeyboardHigh, PMidiChannel, PLevel, PPan,

        PVco1On,
        PVco1Octave, PVco1Tune, PVco1WaveAngle, PVco1Shape, PVco1Pw, PVco1Level,
        PVco2Octave, PVco2Tune, PVco2WaveAngle, PVco2Shape, PVco2Pw, PVco2Level, PVco2Sync,
        PVco3Octave, PVco3Tune, PVco3WaveAngle, PVco3Shape, PVco3Pw, PVco3Level, PVco3Sync,
        PVco3Fm,

        PMixNoise, PMixNoiseColor, PMixRing, PMixExt,

        PSvfCutoff, PSvfResonance, PSvfMode, PSvfSlope, PSvfEnvAmt, PSvfKeyTrack, PSvfLevel,
        PLadderCutoff, PLadderResonance, PLadderDrive, PLadderSlope, PLadderEnvAmt,
        PLadderKeyTrack,
        PFiltersRouting, PFiltersMix,

        PEnv1Delay, PEnv1Attack, PEnv1Hold, PEnv1Decay, PEnv1Sustain, PEnv1Release,
        PEnv1Velocity, PEnv1Loop,
        PEnv2Delay, PEnv2Attack, PEnv2Hold, PEnv2Decay, PEnv2Sustain, PEnv2Release,
        PEnv2Velocity, PEnv2Loop,
        PEnv3Delay, PEnv3Attack, PEnv3Hold, PEnv3Decay, PEnv3Sustain, PEnv3Release,
        PEnv3Velocity,

        PLfo1Rate, PLfo1Wave, PLfo1Sync, PLfo1Retrig, PLfo1Fade,
        PLfo2Rate, PLfo2Wave, PLfo2Sync, PLfo2Retrig, PLfo2Fade,
        PLfo3Rate, PLfo3Wave, PLfo3Sync, PLfo3Retrig, PLfo3Fade,
        PLfo4Rate, PLfo4Wave, PLfo4Sync, PLfo4Retrig, PLfo4Fade,

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
        PFxDelayTime, PFxDelayFeedback, PFxDelayMix,
        PFxReverbSend,

        PArpEnabled, PArpMode, PArpRate, PArpOctaves, PArpHold, PArpLength,

        PVco1BeatHz, PVco2BeatHz, PVco3BeatHz,
        PVoiceMode, PUnisonVoices, PUnisonDetune,
        PParamCount
    };

    enum MasterParam
    {
        MMasterVolume, MReverbSize, MReverbDamping, MReverbMix, MPitchBendRange,
        MDrift, MSelectedPart, MVoices, MGlide, MAftertouch,
        MParamCount
    };

    enum ModSource
    {
        MsNone = 0, MsLfo1, MsLfo2, MsLfo3, MsLfo4, MsEnv1, MsEnv2, MsEnv3,
        MsVelocity, MsAftertouch, MsModWheel, MsKeyTrack, MsRandom, MsCount
    };

    enum ModDest
    {
        MdNone = 0, MdVco1Pitch, MdVco2Pitch, MdVco3Pitch, MdPw,
        MdOsc1Level, MdOsc2Level, MdOsc3Level, MdNoiseLevel,
        MdSvfCutoff, MdSvfResonance, MdLadderCutoff, MdLadderResonance,
        MdFilterMix, MdPan, MdVolume, MdCount
    };

    // -----------------------------------------------------------------------
    // Runtime state
    // -----------------------------------------------------------------------
    struct Voice
    {
        bool active = false;
        bool gated = false;
        int part = 0;
        int note = 60;
        int channel = 1;
        float velocity = 0.8f;
        uint64_t serial = 0;

        double pitch = 60.0;
        double targetPitch = 60.0;
        float drift = 0.0f;
        float driftTarget = 0.0f;
        int driftCountdown = 0;
        float randomValue = 0.0f;
        float fmSample = 0.0f;
        float unisonPosition = 0.0f;
        float unisonGain = 1.0f;
        std::array<float, kEnvCount> lastEnv {};

        std::array<Oscillator, 3> oscillators {};
        std::array<DahdsrEnvelope, kEnvCount> envelopes {};
        std::array<StateVariableFilter, 2> svf {};
        LadderFilter ladder {};
        NoiseSource noise {};
        Rng rng { 0x1f2e3d4cu };
    };

    struct PartRuntime
    {
        std::array<Lfo, kLfoCount> lfos {};
        std::array<float, kLfoCount> lfoValue {};
        std::array<int, kLfoCount> lfoFadeTimer {};
        std::array<int, kLfoCount> lfoFadeLength {};

        StereoDelay delay {};
        ChorusEffect chorus {};

        Smoother levelSmoother {};
        Smoother panSmoother {};
        Smoother mixSmoother {};
        Smoother svfCutoffSmoother {};
        Smoother ladderCutoffSmoother {};

        int lastNote = 60;
        float busL = 0.0f;
        float busR = 0.0f;
    };

    struct HeldNote
    {
        int channel = 1;
        int note = 60;
        float velocity = 0.8f;
        bool sustained = false;
        unsigned int partMask = 0;
    };

    // Definition and cached parameter tables.
    void buildDefinition();
    void addPartDefinition (int part);
    void addArpeggiatorDefinition (int part);
    void addMasterDefinition();
    void cacheDefaults();

    float value (int part, PartParam id) const
    {
        return values_[(size_t) partIndex_[(size_t) part][(size_t) id]];
    }

    float masterValue (MasterParam id) const
    {
        return values_[(size_t) masterIndex_[(size_t) id]];
    }

    float praw (int part, PartParam id) const
    {
        return partRaw_[(size_t) part][(size_t) id];
    }

    static int choiceOf (float value, int maxIndex);
    static float clamp01 (float value) { return clampf (value, 0.0f, 1.0f); }

    // MIDI routing helpers.
    int normalizeChannel (int channel) const;
    int partChannel (int part) const;
    int selectedPartIndex() const;
    bool partNoteInRange (int part, int note) const;
    int targetParts (int channel, int note, int* outParts) const;

    void addHeldNote (int channel, int note, float velocity, unsigned int partMask);
    void removeHeldNote (int index);
    void releaseVoicesFor (int channel, int note);
    void releaseAllGatedVoices();
    void killAllVoices();
    void noteOffInternal (int channel, int note);
    void setSustain (int channel, bool down);

    // Voice management.
    void startVoice (int part, int note, float velocity, int channel,
                     float unisonPosition = 0.0f, float unisonGain = 1.0f);
    void startPartNote (int part, int note, float velocity, int channel);
    void rebuildPartVoices (int part);
    void updateMonoPart (int part);
    void killVoice (Voice& voice);
    void killPartVoices (int part);
    int voiceMode (int part) const;
    int lastHeldForPart (int part) const;
    void enforceVoiceBudget();
    int allocateVoiceSlot();
    void releaseVoice (Voice& voice);

    // DSP helpers.
    StereoSample renderVoice (Voice& voice, int sampleIndex);
    float lfoRateFor (int part, int lfo) const;

    // Static tables used by the hot path (defined in the .cpp).
    static constexpr int kEnvBase[kEnvCount] = { PEnv1Delay, PEnv2Delay, PEnv3Delay };

    ModelDefinition definition_;
    std::array<std::array<int, PParamCount>, kPartCount> partIndex_ {};
    std::array<int, MParamCount> masterIndex_ {};
    std::vector<float> values_;

    // Per-block parameter snapshots (render never touches the strings).
    std::array<std::array<float, PParamCount>, kPartCount> partRaw_ {};
    std::array<float, MParamCount> masterRaw_ {};
    std::array<std::array<float, 3>, kPartCount> vcoRatio_ {};

    TransportInfo transport_ {};
    double sampleRate_ = 48000.0;
    int maxBlockSize_ = 512;
    bool prepared_ = false;

    std::array<Voice, kMaxVoices> voices_ {};
    uint64_t voiceSerial_ = 0;
    int activeVoices_ = 0;
    int voiceBudget_ = 8;

    std::array<PartRuntime, kPartCount> parts_ {};
    std::array<bool, kPartCount> enabledPrev_ {};

    std::array<HeldNote, kMaxHeldNotes> held_ {};
    int heldCount_ = 0;
    std::array<bool, 16> sustainPedal_ {};

    float pitchBend_ = 0.0f;
    float modWheel_ = 0.0f;
    float aftertouch_ = 0.0f;

    // Per-sample part scratch values shared by the voice renderer.
    std::array<std::array<float, kLfoCount>, kPartCount> lfoNow_ {};
    std::array<float, kPartCount> partGain_ {};
    std::array<float, kPartCount> partPan_ {};
    std::array<float, kPartCount> svfBase_ {};
    std::array<float, kPartCount> ladderBase_ {};
    std::array<float, kPartCount> filterMix_ {};

    ReverbEffect reverb_ {};
    Smoother masterSmoother_ {};
    DCBlocker dcLeft_ {}, dcRight_ {};
    float meter_ = 0.0f;

    float glideCoeff_ = 1.0f;
    float driftCoeff_ = 0.001f;
    int driftInterval_ = 1;
    float driftAmount_ = 0.0f;
    float aftertouchDepth_ = 0.0f;
    float bendSemitones_ = 2.0f;

    const float* externalInput_ = nullptr;
    int externalInputSize_ = 0;
};

std::unique_ptr<SynthEngine> makeOneEngine();

} // namespace triad
