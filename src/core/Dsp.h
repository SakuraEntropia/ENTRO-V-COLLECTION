#pragma once

#include "core/Engine.h"

#include <array>
#include <cstdint>
#include <vector>

namespace triad
{

// ---------------------------------------------------------------------------
// Utilities
// ---------------------------------------------------------------------------

/** One-pole parameter smoother. */
class Smoother
{
public:
    void prepare (double sampleRate, float timeMs = 12.0f);
    void setTarget (float target) { target_ = target; }
    void snap (float value) { current_ = target_ = value; }
    float next() noexcept
    {
        current_ += (target_ - current_) * coefficient_;
        return current_;
    }
    float current() const noexcept { return current_; }
    bool isSettled() const noexcept
    {
        return std::abs (target_ - current_) < 1.0e-5f;
    }

private:
    float current_ = 0.0f;
    float target_ = 0.0f;
    float coefficient_ = 0.01f;
};

/** Removes the DC offset produced by asymmetric waveshaping. */
class DCBlocker
{
public:
    void prepare (double sampleRate, float cutoffHz = 12.0f);
    float process (float x) noexcept;
    void reset() { x1_ = y1_ = 0.0f; }

private:
    float r_ = 0.999f;
    float x1_ = 0.0f;
    float y1_ = 0.0f;
};

// ---------------------------------------------------------------------------
// Oscillators
// ---------------------------------------------------------------------------

class Oscillator
{
public:
    enum class Wave { Sine, Triangle, Saw, Square, Pulse };

    void prepare (double sampleRate);
    void reset (double phase = 0.0) noexcept { phase_ = phase; }
    void resetPhase() noexcept { phase_ = 0.0; }

    void setFrequency (double hz) noexcept;
    double frequency() const noexcept { return frequency_; }

    /** Advances one sample. Returns true when the phase wraps. */
    bool advance() noexcept;

    void setPhase (double phase) noexcept { phase_ = phase; }
    double phase() const noexcept { return phase_; }
    double increment() const noexcept { return increment_; }

    float sample (Wave wave, float pulseWidth = 0.5f) const noexcept;

    /** Normalised BLEP residual for an externally forced reset (sync). */
    static double polyBlep (double t, double dt) noexcept;

private:
    double sampleRate_ = 48000.0;
    double phase_ = 0.0;
    double increment_ = 0.0;
    double frequency_ = 440.0;
};

// ---------------------------------------------------------------------------
// Filters
// ---------------------------------------------------------------------------

/** Four-pole transistor-ladder approximation with nonlinear feedback.
    Outputs are tapped per stage so 6/12/18/24 dB slopes are available. */
class LadderFilter
{
public:
    void prepare (double sampleRate);
    void reset();
    void setCutoffHz (float hz);
    void setResonance01 (float resonance);
    void setDrive (float drive);
    void setSlope (int poles); // 1..4

    float process (float input) noexcept;
    float lastOutput() const noexcept { return lastOut_; }

private:
    void updateCoefficients();

    double sampleRate_ = 48000.0;
    float cutoffHz_ = 1000.0f;
    float resonance_ = 0.0f;
    float drive_ = 1.0f;
    int poles_ = 4;
    float g_ = 0.0f;
    float bigG_ = 0.0f;
    float k_ = 0.0f;
    float stage_[4] {};
    float lastOut_ = 0.0f;
    float lastCutoff_ = -1.0f;
};

/** Topology-preserving state variable filter (12 dB/oct). */
class StateVariableFilter
{
public:
    enum class Mode { LowPass, HighPass, BandPass, Notch };

    void prepare (double sampleRate);
    void reset();
    void setCutoffHz (float hz);
    void setResonance01 (float resonance);
    float process (float input, Mode mode) noexcept;

private:
    void update();

    double sampleRate_ = 48000.0;
    float cutoffHz_ = 1000.0f;
    float resonance_ = 0.0f;
    float g_ = 0.0f;
    float k_ = 1.0f;
    float a1_ = 0.0f;
    float a2_ = 0.0f;
    float a3_ = 0.0f;
    float ic1_ = 0.0f;
    float ic2_ = 0.0f;
    float lastCutoff_ = -1.0f;
    float lastResonance_ = -1.0f;
};

/** Smoothly variable multimode filter used for the Brute "Steiner" model. */
class SteinerFilter
{
public:
    void prepare (double sampleRate);
    void reset();
    void setCutoffHz (float hz);
    void setResonance01 (float resonance);
    /** 0 = LP, 1 = notch, 2 = HP, 3 = BP. Values in between blend. */
    float process (float input, float modePosition) noexcept;

private:
    StateVariableFilter filter_;
};

// ---------------------------------------------------------------------------
// Envelopes
// ---------------------------------------------------------------------------

class Envelope
{
public:
    void prepare (double sampleRate);
    void setParameters (float attackSec, float decaySec, float sustain01, float releaseSec);
    void noteOn (bool retrigger = true);
    void noteOff();
    void reset();

    float process() noexcept;
    float level() const noexcept { return level_; }
    bool isActive() const noexcept { return stage_ != Stage::Idle; }
    bool isReleasing() const noexcept { return stage_ == Stage::Release; }

private:
    enum class Stage { Idle, Attack, Decay, Sustain, Release };

    float coefficient (float seconds, float spread) const;

    double sampleRate_ = 48000.0;
    float attack_ = 0.005f;
    float decay_ = 0.2f;
    float sustain_ = 0.7f;
    float release_ = 0.3f;
    float level_ = 0.0f;
    Stage stage_ = Stage::Idle;
};

/** Delay / attack / hold / decay / sustain / release, used by Moog One. */
class DahdsrEnvelope
{
public:
    void prepare (double sampleRate);
    void setParameters (float delaySec, float attackSec, float holdSec,
                        float decaySec, float sustain01, float releaseSec);
    void noteOn (bool retrigger = true);
    void noteOff();
    void reset();
    float process() noexcept;
    bool isActive() const noexcept { return stage_ != Stage::Idle; }
    float level() const noexcept { return level_; }

private:
    enum class Stage { Idle, Delay, Attack, Hold, Decay, Sustain, Release };

    float coefficient (float seconds, float spread) const;

    double sampleRate_ = 48000.0;
    float delay_ = 0.0f;
    float attack_ = 0.005f;
    float hold_ = 0.0f;
    float decay_ = 0.2f;
    float sustain_ = 0.7f;
    float release_ = 0.3f;
    float level_ = 0.0f;
    float timer_ = 0.0f;
    Stage stage_ = Stage::Idle;
};

// ---------------------------------------------------------------------------
// Modulation sources
// ---------------------------------------------------------------------------

enum class LfoWave { Sine, Triangle, SawUp, SawDown, Square, SampleHold, RandomSmooth };

class Lfo
{
public:
    void prepare (double sampleRate, uint32_t seed = 0x51ed270bu);
    void setRateHz (float hz);
    void setWave (LfoWave wave) { wave_ = wave; }
    /** 0..1 offset added to the oscillator phase. */
    void setPhaseOffset (float cycles) { offset_ = cycles; }
    void setBipolar (bool bipolar) { bipolar_ = bipolar; }
    void reset (float startPhase = 0.0f);
    float process() noexcept;
    float value() const noexcept { return last_; }
    bool wrappedThisSample() const noexcept { return wrapped_; }

private:
    double sampleRate_ = 48000.0;
    double phase_ = 0.0;
    double increment_ = 0.0;
    double rateHz_ = 1.0;
    double offset_ = 0.0;
    LfoWave wave_ = LfoWave::Sine;
    bool bipolar_ = true;
    bool wrapped_ = false;
    float sampleHold_ = 0.0f;
    float randomTarget_ = 0.0f;
    float smoothNoise_ = 0.0f;
    float last_ = 0.0f;
    Rng rng_ { 0x51ed270bu };
};

class NoiseSource
{
public:
    void prepare (double sampleRate, uint32_t seed = 0xabcdef01u);
    void reset();
    float white() noexcept;
    float pink() noexcept;

private:
    Rng rng_;
    float b0_ = 0.0f, b1_ = 0.0f, b2_ = 0.0f, b3_ = 0.0f, b4_ = 0.0f, b5_ = 0.0f, b6_ = 0.0f;
};

// ---------------------------------------------------------------------------
// Effects
// ---------------------------------------------------------------------------

class StereoDelay
{
public:
    void prepare (double sampleRate, float maxDelaySeconds = 2.5f);
    void reset();
    void setDelaySeconds (float leftSeconds, float rightSeconds);
    void setFeedback (float feedback01);
    void setMix (float mix01);
    void setPingPong (bool enabled) { pingPong_ = enabled; }
    void setDampingHz (float hz);
    void setModulation (float depth01, float rateHz);
    StereoSample process (float inputLeft, float inputRight) noexcept;

private:
    float read (const std::vector<float>& buffer, float delaySamples) const noexcept;

    double sampleRate_ = 48000.0;
    std::vector<float> bufferLeft_;
    std::vector<float> bufferRight_;
    int writeIndex_ = 0;
    int size_ = 1;
    float targetLeft_ = 0.0f;
    float targetRight_ = 0.0f;
    float delayLeft_ = 0.0f;
    float delayRight_ = 0.0f;
    float feedback_ = 0.35f;
    float mix_ = 0.25f;
    bool pingPong_ = false;
    float dampCoefficient_ = 0.5f;
    float dampLeft_ = 0.0f;
    float dampRight_ = 0.0f;
    float modDepth_ = 0.0f;
    float modRate_ = 0.3f;
    float modPhase_ = 0.0f;
    float modIncrement_ = 0.0f;
};

class ChorusEffect
{
public:
    void prepare (double sampleRate);
    void reset();
    void setRateHz (float hz);
    void setDepth (float depth01);
    void setMix (float mix01);
    StereoSample process (float inputLeft, float inputRight) noexcept;

private:
    float read (const std::vector<float>& buffer, float delaySamples) const noexcept;

    double sampleRate_ = 48000.0;
    std::vector<float> bufferLeft_;
    std::vector<float> bufferRight_;
    int writeIndex_ = 0;
    int size_ = 1;
    float phase_ = 0.0f;
    float increment_ = 0.0f;
    float depth_ = 0.3f;
    float mix_ = 0.4f;
};

class PhaserEffect
{
public:
    void prepare (double sampleRate);
    void reset();
    void setRateHz (float hz);
    void setDepth (float depth01);
    void setFeedback (float feedback01);
    void setMix (float mix01);
    StereoSample process (float inputLeft, float inputRight) noexcept;

private:
    void updateCoefficients (float lfo);

    double sampleRate_ = 48000.0;
    float phase_ = 0.0f;
    float increment_ = 0.0f;
    float depth_ = 0.6f;
    float feedback_ = 0.4f;
    float mix_ = 0.5f;
    float lastLeft_ = 0.0f;
    float lastRight_ = 0.0f;
    float coefLeft_[6] {};
    float coefRight_[6] {};
    float stateLeft_[6] {};
    float stateRight_[6] {};
};

class ConvolutionEffect
{
public:
    void prepare (double sampleRate, int seed);
    void reset();
    void setMix (float mix01);
    StereoSample process (float inputLeft, float inputRight) noexcept;

private:
    double sampleRate_ = 48000.0;
    std::array<std::vector<float>, 2> impulse_;
    std::array<std::vector<float>, 2> buffer_;
    int writeIndex_ = 0;
    int size_ = 1;
    float mix_ = 0.0f;
};

class ReverbEffect
{
public:
    void prepare (double sampleRate);
    void reset();
    void setSize (float size01);
    void setDamping (float damping01);
    void setMix (float mix01);
    StereoSample process (float inputLeft, float inputRight) noexcept;

private:
    struct DelayLine
    {
        std::vector<float> buffer;
        int writeIndex = 0;
        float damping = 0.0f;
    };

    int read (const DelayLine& line) const noexcept;

    double sampleRate_ = 48000.0;
    std::array<DelayLine, 4> lines_;
    float mix_ = 0.25f;
    float decay_ = 0.8f;
    float damp_ = 0.3f;
};

enum class DistortionType { Soft, Hard, Fold, Tube };

float applyDistortion (float input, DistortionType type, float drive) noexcept;
float softClip (float input) noexcept;

} // namespace triad
