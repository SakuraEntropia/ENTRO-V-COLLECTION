#pragma once

#include "core/Dsp.h"
#include "core/Engine.h"
#include "core/ContinuumProcessing.h"
#include <mutex>

#include <array>
#include <atomic>
#include <cstdint>

namespace triad
{

/** CONTINUUM V — eight-voice continuous-surface instrument with an original
    matrix engine called EIGEN MATRIX.

    The matrix follows the EaganMatrix patching idea: sources run down the
    left, destinations across the top, and every intersection carries a formula
    and an amount.  Audio sources route into filters/master; shape generators
    route into pitch/cutoff. */
class ContinuumEngine final : public SynthEngine
{
public:
    static constexpr int kVoices = 8;
    static constexpr int kOscCount = 5;
    static constexpr int kFilterCount = 5;
    static constexpr int kBankCount = 2;
    static constexpr int kDelayCount = 2;
    static constexpr int kSgCount = 5;
    static constexpr int kEnvCount = 2;
    static constexpr int kSources = 16;
    static constexpr int kDestinations = 22;
    static constexpr int kFormulaCount = 19;

    ContinuumEngine();

    const ModelDefinition& definition() const override { return definition_; }
    void prepare (double sampleRate, int maxBlockSize) override;
    void reset() override;
    void setParameters (const float* values, int count) override;
    void processMidi (const MidiEvent& event) override;
    void render (float* left, float* right, int numSamples) override;

    int activeVoiceCount() const override;
    float outputLevel() const override { return meter_; }

    // EIGEN MATRIX ----------------------------------------------------------
    int matrixRows() const override { return kSources; }
    int matrixColumns() const override { return kDestinations; }
    std::string matrixSourceName (int source) const override;
    std::string matrixDestinationName (int destination) const override;
    int matrixFormulaCount() const override { return kFormulaCount; }
    std::string matrixFormulaName (int formula) const override;
    void setMatrixCell (int source, int destination, int formula, float amount) override;
    int matrixCellFormula (int source, int destination) const override;
    float matrixCellAmount (int source, int destination) const override;
    void resetMatrix() override;

    void setNoteExpression (int note, float pitchSemitones,
                            float timbre01, float pressure01) override;
    void setUserFormula (const std::string& formula) override;

    bool setFormulaText (int slot, const std::string& text) override;
    std::string formulaText (int slot) const override;
    float evaluateNamedFormula (int slot, float source, float x, float y, float z);
    static float transfer (float input, int curve);

    struct UserFormula
    {
        enum Op : uint8_t
        {
            Const, Source, X, Y, Z, Add, Sub, Mul, Div, Pow,
            Neg, Sin, Cos, Tan, Abs, Sqrt, Exp, Log, Min, Max
        };

        std::array<uint8_t, 128> ops {};
        std::array<float, 64> constants {};
        int opCount = 0;
        bool valid = false;

        float eval (float source, float x, float y, float z) const;
    };

private:
    enum ScalarParam
    {
        PMasterVolume, PMasterGlide, PMasterY, PMasterZ, PMasterBendRange,
        PMasterNoise, PMasterReverbMix, PMasterReverbSize,
        PMasterDelayMix, PMasterDelayTime, PMasterDelayFeedback,
        PCvc1, PCvc2, PCvc3, PCvc4,
        PSurfaceQuantize,
        PFxMode, PFxReverbDamping, PFxConv1Mix, PFxConv2Mix,
        PFxDelayLpf, PFxDelayOffset, PFxDelayPingPong, PFxDelayFeedback,
        PFxDelayFull, PFxDelayTime,
        PFxChorusRate, PFxChorusDepth, PFxChorusMix,
        PFxPhaserRate, PFxPhaserDepth, PFxPhaserFeedback, PFxPhaserMix,
        PFxDelayModDepth, PFxDelayModRate,
        PNoiseLevel, PNoiseColor,
        PCompEnabled, PCompThreshold, PCompRatio, PCompAttack, PCompRelease, PCompMakeup,
        PTilt, PTiltFrequency, PPreGain, PPostGain,
        PScalarParamCount
    };

    enum OscParam { OscRatio, OscFine, OscPhase, OscShape, OscLevel, OscWave, OscParamCount };
    enum FilterParam { FltType, FltCutoff, FltResonance, FltEnv, FltModel, FltStages, FilterParamCount };
    enum BankParam { BankFreq, BankSpread, BankBandwidth, BankMix, BankLevel, BankKeyTrack, BankSize, BankBWSpread, BankCentre, BankWeight, BankParamCount };
    enum DelayParam { DelayTime, DelayFeedback, DelayMix, DelayParamCount };
    enum SgParam { SgRate, SgShape, SgLevel, SgCycle, SgTrigger, SgParamCount };
    enum EnvParam { EnvAttack, EnvDecay, EnvSustain, EnvRelease, EnvParamCount };

    struct ShapeGenerator
    {
        double phase = 0.0;
        float last = 0.0f;
        float sampleHold = 0.0f;
        bool finished = false, triggerHigh = false;
        Rng rng { 0x6d2b79f5u };
    };

    struct Voice
    {
        bool active = false;
        bool held = false;
        int note = -1;
        int channel = 0;
        int age = 0;
        float velocity = 0.8f;
        float exprPitch = 0.0f;
        float bend = 0.0f;
        float timbre = 0.5f;
        float pressure = 0.5f;
        float targetHz = 440.0f;
        float currentHz = 440.0f;

        std::array<Oscillator, kOscCount> oscillators;
        std::array<StateVariableFilter, kFilterCount> filters;
        std::array<float, kFilterCount> lastFilter {};
        std::array<std::array<StateVariableFilter, 3>, kFilterCount> extraFilters;
        std::array<StateVariableFilter, 48> bank1;
        std::array<StateVariableFilter, 48> bank2;
        std::array<ShapeGenerator, kSgCount> shapes;
        std::array<Envelope, kEnvCount> envelopes;
    };

    void buildDefinition();
    float param (ScalarParam id) const { return values_[(size_t) scalarIndex_[(size_t) id]]; }
    int choice (ScalarParam id) const { return (int) std::lround (param (id)); }
    float ovalue (int osc, OscParam which) const { return values_[(size_t) oscIndex_[(size_t) osc][(size_t) which]]; }
    float fvalue (int filter, FilterParam which) const { return values_[(size_t) filterIndex_[(size_t) filter][(size_t) which]]; }
    float bvalue (int bank, BankParam which) const { return values_[(size_t) bankIndex_[(size_t) bank][(size_t) which]]; }
    float dvalue (int delay, DelayParam which) const { return values_[(size_t) delayIndex_[(size_t) delay][(size_t) which]]; }
    float svalue (int sg, SgParam which) const { return values_[(size_t) sgIndex_[(size_t) sg][(size_t) which]]; }
    float evalue (int env, EnvParam which) const { return values_[(size_t) envIndex_[(size_t) env][(size_t) which]]; }

    void setMatrixDefault();
    void refreshMatrix();
    static uint32_t packCell (int formula, float amount);
    static int cellFormula (uint32_t packed) { return (int) (packed & 0xffu); }
    static float cellAmount (uint32_t packed)
    {
        return (float) ((packed >> 8) & 0xffffu) / 65535.0f;
    }

    float evalFormula (int formula, float source, float x, float y, float z);

    int allocateVoice (int note, int channel);
    Voice* findVoice (int note);
    template <typename Fn>
    void updateVoicesForNote (int note, Fn&& fn);
    void startVoice (Voice& voice, int note, int channel, float velocity);
    void releaseVoice (Voice& voice);
    void updateChannelExpression (int channel);
    Voice* findVoiceByChannel (int channel);

    ModelDefinition definition_;
    std::array<int, PScalarParamCount> scalarIndex_ {};
    std::array<std::array<int, OscParamCount>, kOscCount> oscIndex_ {};
    std::array<std::array<int, FilterParamCount>, kFilterCount> filterIndex_ {};
    std::array<std::array<int, BankParamCount>, kBankCount> bankIndex_ {};
    std::array<std::array<int, DelayParamCount>, kDelayCount> delayIndex_ {};
    std::array<std::array<int, SgParamCount>, kSgCount> sgIndex_ {};
    std::array<std::array<int, EnvParamCount>, kEnvCount> envIndex_ {};
    std::vector<float> values_;
    std::vector<float> defaults_;

    double sampleRate_ = 48000.0;
    int maxBlock_ = 512;
    uint64_t ageCounter_ = 0;
    float meter_ = 0.0f;
    mutable std::mutex formulaMutex_;
    std::array<UserFormula, 7> pendingFormulas_ {}, renderFormulas_ {};
    std::array<std::string, 7> formulaTexts_ {};
    std::array<std::array<int, 3>, 6> formulaIndices_ {};
    std::array<std::array<int, 2>, 6> masterIndices_ {};
    std::array<std::array<int, 4>, 2> irIndices_ {};
    ContinuumMasterProcessor masterProcessor_;

    std::array<Voice, kVoices> voices_ {};
    std::array<float, 16> channelBend_ {};
    std::array<float, 16> channelTimbre_ {};
    std::array<float, 16> channelPressure_ {};

    std::array<std::atomic<uint32_t>, kSources * kDestinations> matrixCells_ {};
    std::atomic<bool> matrixDirty_ { true };
    std::array<uint8_t, kSources * kDestinations> matrixFormula_ {};
    std::array<float, kSources * kDestinations> matrixAmount_ {};

    NoiseSource noise_;
    StereoDelay delay_;
    StereoDelay delayBank1_;
    StereoDelay delayBank2_;
    ConvolutionEffect convPre_;
    ConvolutionEffect convPost_;
    ChorusEffect chorus_;
    PhaserEffect phaser_;
    ReverbEffect reverb_;
    Smoother volumeSmoother_;
    DCBlocker dcLeft_, dcRight_;

    float lastNoise_ = 0.0f;
    float lastGain_ = 0.0f;
};

std::unique_ptr<SynthEngine> makeContinuumEngine();

} // namespace triad
