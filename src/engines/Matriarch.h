#pragma once

#include "core/Dsp.h"
#include "core/Engine.h"

#include <array>
#include <atomic>
#include <memory>

namespace triad
{

/** Matriarch: four-oscillator paraphonic semi-modular engine.
    The normalized signal path is evaluated per sample; patch cables replace the
    normalled connections at the corresponding jacks. */
class MatriarchEngine final : public SynthEngine
{
public:
    MatriarchEngine();

    const ModelDefinition& definition() const override { return definition_; }

    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void setParameters (const float* values, int count) override;
    void setTransport (const TransportInfo& transport) override;
    void processMidi (const MidiEvent& event) override;
    void render (float* left, float* right, int numSamples) override;

    int activeVoiceCount() const override { return heldCount_ > 0 ? 1 : 0; }
    float outputLevel() const override { return meter_; }

    bool supportsPatching() const override { return true; }
    void setPatchCable (int fromOutput, int toInput, bool connected) override;
    void clearPatchCables() override;

private:
    enum ParamId
    {
        PMasterVolume, PVcaMode, PParaphony, PVelocity, PAftertouch, PFineTune,
        PVco1Octave, PVco1Wave,
        PVco2Octave, PVco2Freq, PVco2Wave, PVco2Sync,
        PVco3Octave, PVco3Freq, PVco3Wave, PVco3Sync,
        PVco4Octave, PVco4Freq, PVco4Wave, PVco4Sync,
        PSyncEnable,
        PMixOsc1, PMixOsc2, PMixOsc3, PMixOsc4, PMixNoise,
        PFilterMode, PFilterCutoff, PFilterSpacing, PFilterRes1, PFilterRes2,
        PFilterEnvAmt, PFilterKbTrack,
        PAmpAttack, PAmpDecay, PAmpSustain, PAmpRelease,
        PFltAttack, PFltDecay, PFltSustain, PFltRelease,
        PLfoRate, PLfoWave, PLfoPitchAmt, PLfoPitchAssign, PLfoCutoffAmt, PLfoPwAmt,
        PLfo2Rate, PLfo2Bipolar,
        PAtten1, PAtten2, PAtten3,
        PDelayTime, PDelaySpacing, PDelayFeedback, PDelayMix, PDelaySync, PDelayPingPong,
        PGlide, PParamCount
    };

    enum JackId
    {
        JOsc1Pitch, JOsc1Fm, JOsc1Pwm, JOsc1Out,
        JOsc2Pitch, JOsc2Fm, JOsc2Pwm, JOsc2Out,
        JOsc3Pitch, JOsc3Fm, JOsc3Pwm, JOsc3Out,
        JOsc4Pitch, JOsc4Fm, JOsc4Pwm, JOsc4Out,
        JMixNoiseIn, JMixOsc1In, JMixOsc2In, JMixOsc3In, JMixOsc4In, JMixOut,
        JFlt1In, JFlt2In, JFlt1Cutoff, JFlt2Cutoff, JFltEnvAmt, JFlt1Out, JFlt2Out,
        JAmpTrig, JAmpOut, JAmpEnd, JFltTrig, JFltOut, JFltEnd,
        JVca1In, JVca2In, JVca1Cv, JVca2Cv,
        JDelayIn1, JDelayIn2, JDelayFbCv, JDelayMixCv, JDelayTime1Cv, JDelayTime2Cv,
        JDelayOut1, JDelayOut2,
        JLfoRateIn, JLfoSyncIn, JLfoOut, JNoiseOut, JShOut,
        JMult1A, JMult1B, JMult1C, JMult1D,
        JMult2A, JMult2B, JMult2C, JMult2D,
        JAtt1In, JAtt1Out, JAtt2In, JAtt2Out, JAtt3In, JAtt3Out,
        JLfo2RateIn, JLfo2Out,
        JClockIn, JClockOut, JStep1, JStep2, JResetIn, JResetOut,
        JKbPitch, JKbGate, JKbVelocity, JKbAftertouch, JKbMod,
        JJackCount
    };

    void buildDefinition();
    void cacheIndices();
    float param (ParamId id) const { return values_[(size_t) paramIndex_[(size_t) id]]; }
    int choice (ParamId id) const { return (int) std::lround (param (id)); }

    float readJack (int input, float normal) const;
    void writeJack (int output, float value);
    void latchPatchCables();
    void updateHeldNotes();

    int soundingNoteFor (int oscillator) const;
    bool oscillatorGate (int oscillator) const;

    ModelDefinition definition_;
    std::array<int, PParamCount> paramIndex_ {};
    std::array<int, JJackCount> jackIndex_ {};
    std::vector<float> values_;
    std::vector<float> defaults_;

    double sampleRate_ = 48000.0;
    int maxBlock_ = 512;

    // Patch matrix: for each input jack a bitmask of connected source jacks.
    std::array<std::atomic<uint64_t>, JJackCount> cableLow_ {};
    std::array<std::atomic<uint64_t>, JJackCount> cableHigh_ {};
    std::array<int16_t, JJackCount> cableSource_ {};

    std::array<float, JJackCount> bus_ {};
    float mult1Bus_ = 0.0f;
    float mult2Bus_ = 0.0f;

    std::array<Oscillator, 4> oscillators_ {};
    std::array<float, 4> oscillatorPhaseIncrement_ {};
    LadderFilter ladder1_, ladder2_;
    StateVariableFilter hp1_[2];
    Envelope ampEnvelope_, filterEnvelope_;
    Lfo lfo_, lfo2_;
    NoiseSource noise_;
    StereoDelay delay_;

    std::array<int, 16> heldNotes_ {};
    std::array<float, 16> heldVelocities_ {};
    int heldCount_ = 0;
    std::array<int, 16> sustainedNotes_ {};
    int sustainedCount_ = 0;
    bool sustainPedal_ = false;

    float pitchBend_ = 0.0f;      // -1..1, +/- 2 semitones
    float modWheel_ = 0.0f;       // 0..1
    float aftertouch_ = 0.0f;     // 0..1
    float lastVelocity_ = 0.8f;
    float currentNote_ = 60.0f;
    std::array<float, 4> glideNotes_ { 60.0f, 60.0f, 60.0f, 60.0f };
    bool glideHasNote_ = false;

    TransportInfo transport_;
    uint64_t clockSamples_ = 0;
    double clockAccumulator_ = 0.0;
    int clockPhase_ = 0;
    float sampleHoldValue_ = 0.0f;
    bool ampGatePrev_ = false;
    bool filterGatePrev_ = false;
    bool lfoSyncPrev_ = false;
    bool retriggerPending_ = false;
    float meter_ = 0.0f;

    Smoother masterSmoother_;
    Smoother cutoffSmoother_;
    DCBlocker dcLeft_, dcRight_;
};

std::unique_ptr<SynthEngine> makeMatriarchEngine();

} // namespace triad
