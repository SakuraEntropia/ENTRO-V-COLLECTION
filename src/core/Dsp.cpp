#include "core/Dsp.h"

#include <algorithm>
#include <cmath>

namespace triad
{

// ---------------------------------------------------------------------------
// Utilities
// ---------------------------------------------------------------------------

void Smoother::prepare (double sampleRate, float timeMs)
{
    const auto seconds = std::max (0.0005f, timeMs * 0.001f);
    coefficient_ = 1.0f - std::exp (-1.0f / (seconds * (float) sampleRate));
}

void DCBlocker::prepare (double sampleRate, float cutoffHz)
{
    const auto rc = 1.0f / (2.0f * kPi * cutoffHz);
    const auto dt = 1.0f / (float) sampleRate;
    r_ = rc / (rc + dt);
}

float DCBlocker::process (float x) noexcept
{
    const auto y = x - x1_ + r_ * y1_;
    x1_ = x;
    y1_ = y;
    return y;
}

// ---------------------------------------------------------------------------
// Oscillator
// ---------------------------------------------------------------------------

void Oscillator::prepare (double sampleRate)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    setFrequency (frequency_);
    reset();
}

void Oscillator::setFrequency (double hz) noexcept
{
    frequency_ = hz;
    const auto limited = clampd (hz, -0.49 * sampleRate_, 0.49 * sampleRate_);
    increment_ = limited / sampleRate_;
}

bool Oscillator::advance() noexcept
{
    phase_ += increment_;
    if (phase_ >= 1.0)
    {
        phase_ -= 1.0;
        return true;
    }

    if (phase_ < 0.0)
    {
        phase_ += 1.0;
        return true;
    }

    return false;
}

double Oscillator::polyBlep (double t, double dt) noexcept
{
    if (dt <= 0.0)
        return 0.0;

    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0;
    }

    if (t > 1.0 - dt)
    {
        t = (t - 1.0) / dt;
        return t * t + t + t + 1.0;
    }

    return 0.0;
}

float Oscillator::sample (Wave wave, float pulseWidth) const noexcept
{
    const auto dt = std::abs (increment_);
    auto p = phase_;

    switch (wave)
    {
        case Wave::Sine:
            return (float) std::sin (kTwoPi * p);

        case Wave::Triangle:
            return (float) (1.0 - 4.0 * std::abs (p - 0.5));

        case Wave::Saw:
            return (float) (2.0 * p - 1.0 - polyBlep (p, dt));

        case Wave::Square:
        case Wave::Pulse:
        {
            const auto width = clampf (pulseWidth, 0.03f, 0.97f);
            auto value = (p < (double) width ? 1.0f : -1.0f);
            value += (float) polyBlep (p, dt);
            auto t2 = p - width;
            if (t2 < 0.0)
                t2 += 1.0;
            value -= (float) polyBlep (t2, dt);
            return value;
        }
    }

    return 0.0f;
}

// ---------------------------------------------------------------------------
// Ladder filter
// ---------------------------------------------------------------------------

void LadderFilter::prepare (double sampleRate)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    reset();
    lastCutoff_ = -1.0f;
    updateCoefficients();
}

void LadderFilter::reset()
{
    stage_[0] = stage_[1] = stage_[2] = stage_[3] = 0.0f;
    lastOut_ = 0.0f;
}

void LadderFilter::setCutoffHz (float hz)
{
    const auto limited = clampf (hz, 18.0f, (float) sampleRate_ * 0.45f);
    if (std::abs (limited - cutoffHz_) > 0.01f)
    {
        cutoffHz_ = limited;
        updateCoefficients();
    }
}

void LadderFilter::setResonance01 (float resonance)
{
    resonance_ = clampf (resonance, 0.0f, 1.0f);
    k_ = resonance_ * 4.05f;
}

void LadderFilter::setDrive (float drive)
{
    drive_ = clampf (drive, 0.25f, 12.0f);
}

void LadderFilter::setSlope (int poles)
{
    poles_ = std::clamp (poles, 1, 4);
}

void LadderFilter::updateCoefficients()
{
    g_ = std::tan (kPi * cutoffHz_ / (float) sampleRate_);
    g_ = clampf (g_, 0.0001f, 8.0f);
    bigG_ = g_ / (1.0f + g_);
}

float LadderFilter::process (float input) noexcept
{
    const auto driven = input * drive_;
    const auto u = std::tanh (driven - k_ * stage_[3]);

    float v = (u - stage_[0]) * bigG_;
    const auto y0 = v + stage_[0];
    stage_[0] = y0 + v;

    v = (y0 - stage_[1]) * bigG_;
    const auto y1 = v + stage_[1];
    stage_[1] = y1 + v;

    v = (y1 - stage_[2]) * bigG_;
    const auto y2 = v + stage_[2];
    stage_[2] = y2 + v;

    v = (y2 - stage_[3]) * bigG_;
    const auto y3 = v + stage_[3];
    stage_[3] = y3 + v;

    switch (poles_)
    {
        case 1: lastOut_ = y0; break;
        case 2: lastOut_ = y1; break;
        case 3: lastOut_ = y2; break;
        default: lastOut_ = y3; break;
    }

    return lastOut_;
}

// ---------------------------------------------------------------------------
// State variable filter
// ---------------------------------------------------------------------------

void StateVariableFilter::prepare (double sampleRate)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    reset();
    lastCutoff_ = -1.0f;
    lastResonance_ = -1.0f;
    update();
}

void StateVariableFilter::reset()
{
    ic1_ = ic2_ = 0.0f;
    onePole_ = 0.0f;
}

void StateVariableFilter::setCutoffHz (float hz)
{
    const auto limited = clampf (hz, 18.0f, (float) sampleRate_ * 0.45f);
    if (std::abs (limited - cutoffHz_) > 0.01f)
    {
        cutoffHz_ = limited;
        update();
    }
}

void StateVariableFilter::setResonance01 (float resonance)
{
    const auto r = clampf (resonance, 0.0f, 1.0f);
    if (std::abs (r - resonance_) > 0.001f)
    {
        resonance_ = r;
        update();
    }
}

void StateVariableFilter::update()
{
    g_ = std::tan (kPi * cutoffHz_ / (float) sampleRate_);
    g_ = clampf (g_, 0.0001f, 8.0f);
    k_ = 2.0f - 1.97f * resonance_;
    k_ = clampf (k_, 0.03f, 2.0f);
    a1_ = 1.0f / (1.0f + g_ * (g_ + k_));
    a2_ = g_ * a1_;
    a3_ = g_ * a2_;
    lastCutoff_ = cutoffHz_;
    lastResonance_ = resonance_;
}

float StateVariableFilter::process (float input, Mode mode) noexcept
{
    const auto x = std::tanh (input);
    const auto v3 = x - ic2_;
    const auto v1 = a1_ * ic1_ + a2_ * v3;
    const auto v2 = ic2_ + a2_ * ic1_ + a3_ * v3;
    ic1_ = 2.0f * v1 - ic1_;
    ic2_ = 2.0f * v2 - ic2_;

    const auto low = v2;
    const auto band = v1;
    const auto high = x - k_ * v1 - v2;

    switch (mode)
    {
        case Mode::LowPass: return low;
        case Mode::HighPass: return high;
        case Mode::BandPass: return band;
        case Mode::Notch: return high + low;
        case Mode::LowShelf: return x + low;
        case Mode::HighShelf: return x + high;
        case Mode::UnityBandPass: return k_ * band;
        case Mode::AllPass: return high + low - k_ * band;
        case Mode::OnePoleLowPass:
        case Mode::OnePoleHighPass:
            onePole_ += (1.0f - std::exp (-2.0f * (float) kPi * cutoffHz_ / (float) sampleRate_)) * (x - onePole_);
            return mode == Mode::OnePoleLowPass ? onePole_ : x - onePole_;
    }

    return low;
}

// ---------------------------------------------------------------------------
// Steiner-style variable multimode filter
// ---------------------------------------------------------------------------

void SteinerFilter::prepare (double sampleRate)
{
    filter_.prepare (sampleRate);
}

void SteinerFilter::reset()
{
    filter_.reset();
}

void SteinerFilter::setCutoffHz (float hz)
{
    filter_.setCutoffHz (hz);
}

void SteinerFilter::setResonance01 (float resonance)
{
    filter_.setResonance01 (resonance);
}

float SteinerFilter::process (float input, float modePosition) noexcept
{
    const auto pos = clampf (modePosition, 0.0f, 3.0f);
    const auto low = filter_.process (input, StateVariableFilter::Mode::LowPass);
    const auto band = filter_.process (input, StateVariableFilter::Mode::BandPass);
    const auto high = filter_.process (input, StateVariableFilter::Mode::HighPass);
    const auto notch = filter_.process (input, StateVariableFilter::Mode::Notch);

    const int index = std::clamp ((int) pos, 0, 2);
    const auto frac = pos - (float) index;
    const float modes[4] = { low, notch, high, band };
    return lerpf (modes[index], modes[index + 1], frac);
}

// ---------------------------------------------------------------------------
// Envelopes
// ---------------------------------------------------------------------------

void Envelope::prepare (double sampleRate)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    reset();
}

void Envelope::setParameters (float attackSec, float decaySec, float sustain01, float releaseSec)
{
    attack_ = std::max (0.0005f, attackSec);
    decay_ = std::max (0.0005f, decaySec);
    sustain_ = clampf (sustain01, 0.0f, 1.0f);
    release_ = std::max (0.0005f, releaseSec);
}

float Envelope::coefficient (float seconds, float spread) const
{
    const auto timeConstant = std::max (0.0005f, seconds / std::max (1.0f, spread));
    return 1.0f - std::exp (-1.0f / (timeConstant * (float) sampleRate_));
}

void Envelope::noteOn (bool retrigger)
{
    if (retrigger && stage_ == Stage::Idle)
        level_ = 0.0f;

    stage_ = Stage::Attack;
}

void Envelope::noteOff()
{
    if (stage_ != Stage::Idle)
        stage_ = Stage::Release;
}

void Envelope::reset()
{
    stage_ = Stage::Idle;
    level_ = 0.0f;
}

float Envelope::process() noexcept
{
    switch (stage_)
    {
        case Stage::Idle:
            return 0.0f;

        case Stage::Attack:
        {
            const auto c = coefficient (attack_, 4.0f);
            level_ += (1.02f - level_) * c;
            if (level_ >= 1.0f)
            {
                level_ = 1.0f;
                stage_ = Stage::Decay;
            }
            break;
        }

        case Stage::Decay:
        {
            const auto c = coefficient (decay_, 4.0f);
            level_ += (sustain_ - level_) * c;
            if (std::abs (level_ - sustain_) < 0.0005f)
            {
                level_ = sustain_;
                stage_ = Stage::Sustain;
            }
            break;
        }

        case Stage::Sustain:
            level_ += (sustain_ - level_) * 0.0005f;
            break;

        case Stage::Release:
        {
            const auto c = coefficient (release_, 4.5f);
            level_ += (0.0f - level_) * c;
            if (level_ < 0.0001f)
            {
                level_ = 0.0f;
                stage_ = Stage::Idle;
            }
            break;
        }
    }

    return level_;
}

// ---------------------------------------------------------------------------
// DAHDSR envelope
// ---------------------------------------------------------------------------

void DahdsrEnvelope::prepare (double sampleRate)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    reset();
}

void DahdsrEnvelope::setParameters (float delaySec, float attackSec, float holdSec,
                                    float decaySec, float sustain01, float releaseSec)
{
    delay_ = std::max (0.0f, delaySec);
    attack_ = std::max (0.0005f, attackSec);
    hold_ = std::max (0.0f, holdSec);
    decay_ = std::max (0.0005f, decaySec);
    sustain_ = clampf (sustain01, 0.0f, 1.0f);
    release_ = std::max (0.0005f, releaseSec);
}

float DahdsrEnvelope::coefficient (float seconds, float spread) const
{
    const auto timeConstant = std::max (0.0005f, seconds / std::max (1.0f, spread));
    return 1.0f - std::exp (-1.0f / (timeConstant * (float) sampleRate_));
}

void DahdsrEnvelope::noteOn (bool retrigger)
{
    if (retrigger && stage_ == Stage::Idle)
        level_ = 0.0f;

    timer_ = 0.0f;
    stage_ = delay_ > 0.0001f ? Stage::Delay : Stage::Attack;
}

void DahdsrEnvelope::noteOff()
{
    if (stage_ != Stage::Idle)
        stage_ = Stage::Release;
}

void DahdsrEnvelope::reset()
{
    stage_ = Stage::Idle;
    level_ = 0.0f;
    timer_ = 0.0f;
}

float DahdsrEnvelope::process() noexcept
{
    switch (stage_)
    {
        case Stage::Idle:
            return 0.0f;

        case Stage::Delay:
            timer_ += 1.0f / (float) sampleRate_;
            if (timer_ >= delay_)
            {
                timer_ = 0.0f;
                stage_ = Stage::Attack;
            }
            break;

        case Stage::Attack:
        {
            const auto c = coefficient (attack_, 4.0f);
            level_ += (1.02f - level_) * c;
            if (level_ >= 1.0f)
            {
                level_ = 1.0f;
                timer_ = 0.0f;
                stage_ = hold_ > 0.0001f ? Stage::Hold : Stage::Decay;
            }
            break;
        }

        case Stage::Hold:
            timer_ += 1.0f / (float) sampleRate_;
            if (timer_ >= hold_)
            {
                timer_ = 0.0f;
                stage_ = Stage::Decay;
            }
            break;

        case Stage::Decay:
        {
            const auto c = coefficient (decay_, 4.0f);
            level_ += (sustain_ - level_) * c;
            if (std::abs (level_ - sustain_) < 0.0005f)
            {
                level_ = sustain_;
                stage_ = Stage::Sustain;
            }
            break;
        }

        case Stage::Sustain:
            level_ += (sustain_ - level_) * 0.0005f;
            break;

        case Stage::Release:
        {
            const auto c = coefficient (release_, 4.5f);
            level_ += (0.0f - level_) * c;
            if (level_ < 0.0001f)
            {
                level_ = 0.0f;
                stage_ = Stage::Idle;
            }
            break;
        }
    }

    return level_;
}

// ---------------------------------------------------------------------------
// LFO
// ---------------------------------------------------------------------------

void Lfo::prepare (double sampleRate, uint32_t seed)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    rng_.seed (seed);
    reset();
}

void Lfo::setRateHz (float hz)
{
    rateHz_ = clampd ((double) hz, 0.01, 120.0);
    increment_ = rateHz_ / sampleRate_;
}

void Lfo::reset (float startPhase)
{
    phase_ = std::fmod ((double) startPhase, 1.0);
    if (phase_ < 0.0)
        phase_ += 1.0;
    wrapped_ = false;
    sampleHold_ = rng_.nextBipolar();
    randomTarget_ = rng_.nextBipolar();
    smoothNoise_ = randomTarget_;
    last_ = 0.0f;
}

float Lfo::process() noexcept
{
    phase_ += increment_;
    wrapped_ = false;
    if (phase_ >= 1.0)
    {
        phase_ -= 1.0;
        wrapped_ = true;
        sampleHold_ = rng_.nextBipolar();
        randomTarget_ = rng_.nextBipolar();
    }

    auto p = phase_ + offset_;
    p -= std::floor (p);

    float value = 0.0f;

    switch (wave_)
    {
        case LfoWave::Sine:        value = (float) std::sin (kTwoPi * p); break;
        case LfoWave::Triangle:    value = (float) (1.0 - 4.0 * std::abs (p - 0.5)); break;
        case LfoWave::SawUp:       value = (float) (2.0 * p - 1.0); break;
        case LfoWave::SawDown:     value = (float) (1.0 - 2.0 * p); break;
        case LfoWave::Square:      value = p < 0.5 ? 1.0f : -1.0f; break;
        case LfoWave::SampleHold:  value = sampleHold_; break;
        case LfoWave::RandomSmooth:
            smoothNoise_ += (randomTarget_ - smoothNoise_) * 0.0025f;
            value = smoothNoise_;
            break;
    }

    if (! bipolar_)
        value = (value + 1.0f) * 0.5f;

    last_ = value;
    return value;
}

// ---------------------------------------------------------------------------
// Noise
// ---------------------------------------------------------------------------

void NoiseSource::prepare (double, uint32_t seed)
{
    rng_.seed (seed);
    reset();
}

void NoiseSource::reset()
{
    b0_ = b1_ = b2_ = b3_ = b4_ = b5_ = b6_ = 0.0f;
}

float NoiseSource::white() noexcept
{
    return rng_.nextBipolar();
}

float NoiseSource::pink() noexcept
{
    const auto whiteSample = rng_.nextBipolar();

    b0_ = 0.99886f * b0_ + whiteSample * 0.0555179f;
    b1_ = 0.99332f * b1_ + whiteSample * 0.0750759f;
    b2_ = 0.96900f * b2_ + whiteSample * 0.1538520f;
    b3_ = 0.86650f * b3_ + whiteSample * 0.3104856f;
    b4_ = 0.55000f * b4_ + whiteSample * 0.5329522f;
    b5_ = -0.7616f * b5_ - whiteSample * 0.0168980f;
    const auto pink = b0_ + b1_ + b2_ + b3_ + b4_ + b5_ + b6_ + whiteSample * 0.5362f;
    b6_ = whiteSample * 0.115926f;
    return pink * 0.16f;
}

// ---------------------------------------------------------------------------
// Stereo delay
// ---------------------------------------------------------------------------

void StereoDelay::prepare (double sampleRate, float maxDelaySeconds)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    size_ = (int) (sampleRate_ * (double) maxDelaySeconds) + 8;
    bufferLeft_.assign ((size_t) size_, 0.0f);
    bufferRight_.assign ((size_t) size_, 0.0f);
    writeIndex_ = 0;
    delayLeft_ = targetLeft_ = 0.0f;
    delayRight_ = targetRight_ = 0.0f;
    dampLeft_ = dampRight_ = 0.0f;
    modPhase_ = 0.0f;
    setDampingHz (6000.0f);
    setModulation (0.0f, 0.3f);
}

void StereoDelay::reset()
{
    std::fill (bufferLeft_.begin(), bufferLeft_.end(), 0.0f);
    std::fill (bufferRight_.begin(), bufferRight_.end(), 0.0f);
    writeIndex_ = 0;
    dampLeft_ = dampRight_ = 0.0f;
    delayLeft_ = targetLeft_;
    delayRight_ = targetRight_;
}

void StereoDelay::setDelaySeconds (float leftSeconds, float rightSeconds)
{
    const auto maxSamples = (float) (size_ - 4);
    targetLeft_ = clampf (leftSeconds * (float) sampleRate_, 1.0f, maxSamples);
    targetRight_ = clampf (rightSeconds * (float) sampleRate_, 1.0f, maxSamples);
    if (delayLeft_ < 1.0f)
        delayLeft_ = targetLeft_;
    if (delayRight_ < 1.0f)
        delayRight_ = targetRight_;
}

void StereoDelay::setFeedback (float feedback01)
{
    feedback_ = clampf (feedback01, 0.0f, 0.98f);
}

void StereoDelay::setMix (float mix01)
{
    mix_ = clampf (mix01, 0.0f, 1.0f);
}

void StereoDelay::setDampingHz (float hz)
{
    const auto coefficient = 1.0f - std::exp (-kTwoPi * clampf (hz, 200.0f, 18000.0f) / (float) sampleRate_);
    dampCoefficient_ = clampf (coefficient, 0.0001f, 1.0f);
}

void StereoDelay::setModulation (float depth01, float rateHz)
{
    modDepth_ = clampf (depth01, 0.0f, 1.0f);
    modRate_ = clampf (rateHz, 0.01f, 8.0f);
    modIncrement_ = kTwoPi * modRate_ / (float) sampleRate_;
}

float StereoDelay::read (const std::vector<float>& buffer, float delaySamples) const noexcept
{
    auto position = (float) writeIndex_ - delaySamples;
    while (position < 0.0f)
        position += (float) size_;
    while (position >= (float) size_)
        position -= (float) size_;

    const auto index0 = (int) position;
    const auto index1 = (index0 + 1) % size_;
    const auto frac = position - (float) index0;
    return lerpf (buffer[(size_t) index0], buffer[(size_t) index1], frac);
}

StereoSample StereoDelay::process (float inputLeft, float inputRight) noexcept
{
    modPhase_ += modIncrement_;
    if (modPhase_ > kTwoPi)
        modPhase_ -= kTwoPi;

    const auto modulation = std::sin (modPhase_) * modDepth_ * 0.003f * (float) sampleRate_;

    delayLeft_ += (targetLeft_ - delayLeft_) * 0.0008f;
    delayRight_ += (targetRight_ - delayRight_) * 0.0008f;

    const auto readLeft = read (bufferLeft_, clampf (delayLeft_ + modulation, 1.0f, (float) (size_ - 4)));
    const auto readRight = read (bufferRight_, clampf (delayRight_ - modulation, 1.0f, (float) (size_ - 4)));

    dampLeft_ += (readLeft - dampLeft_) * dampCoefficient_;
    dampRight_ += (readRight - dampRight_) * dampCoefficient_;

    const auto feedbackLeft = pingPong_ ? dampRight_ * feedback_ : dampLeft_ * feedback_;
    const auto feedbackRight = pingPong_ ? dampLeft_ * feedback_ : dampRight_ * feedback_;

    bufferLeft_[(size_t) writeIndex_] = inputLeft + feedbackLeft;
    bufferRight_[(size_t) writeIndex_] = inputRight + feedbackRight;

    writeIndex_ = (writeIndex_ + 1) % size_;

    StereoSample out;
    out.left = inputLeft * (1.0f - mix_) + readLeft * mix_;
    out.right = inputRight * (1.0f - mix_) + readRight * mix_;
    return out;
}

// ---------------------------------------------------------------------------
// Chorus
// ---------------------------------------------------------------------------

void ChorusEffect::prepare (double sampleRate)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    size_ = (int) (sampleRate_ * 0.06) + 8;
    bufferLeft_.assign ((size_t) size_, 0.0f);
    bufferRight_.assign ((size_t) size_, 0.0f);
    writeIndex_ = 0;
    phase_ = 0.0f;
    setRateHz (0.4f);
    setDepth (0.35f);
    setMix (0.4f);
}

void ChorusEffect::reset()
{
    std::fill (bufferLeft_.begin(), bufferLeft_.end(), 0.0f);
    std::fill (bufferRight_.begin(), bufferRight_.end(), 0.0f);
    writeIndex_ = 0;
    phase_ = 0.0f;
}

void ChorusEffect::setRateHz (float hz)
{
    increment_ = kTwoPi * clampf (hz, 0.01f, 8.0f) / (float) sampleRate_;
}

void ChorusEffect::setDepth (float depth01)
{
    depth_ = clampf (depth01, 0.0f, 1.0f);
}

void ChorusEffect::setMix (float mix01)
{
    mix_ = clampf (mix01, 0.0f, 1.0f);
}

float ChorusEffect::read (const std::vector<float>& buffer, float delaySamples) const noexcept
{
    auto position = (float) writeIndex_ - delaySamples;
    while (position < 0.0f)
        position += (float) size_;
    while (position >= (float) size_)
        position -= (float) size_;

    const auto index0 = (int) position;
    const auto index1 = (index0 + 1) % size_;
    const auto frac = position - (float) index0;
    return lerpf (buffer[(size_t) index0], buffer[(size_t) index1], frac);
}

StereoSample ChorusEffect::process (float inputLeft, float inputRight) noexcept
{
    phase_ += increment_;
    if (phase_ > kTwoPi)
        phase_ -= kTwoPi;

    const auto baseSamples = 0.012f * (float) sampleRate_;
    const auto depthSamples = depth_ * 0.006f * (float) sampleRate_;

    const auto modLeft = baseSamples + depthSamples * (0.5f + 0.5f * std::sin (phase_));
    const auto modRight = baseSamples + depthSamples * (0.5f + 0.5f * std::sin (phase_ + kPi * 0.5f));

    const auto wetLeft = read (bufferLeft_, modLeft);
    const auto wetRight = read (bufferRight_, modRight);

    bufferLeft_[(size_t) writeIndex_] = inputLeft;
    bufferRight_[(size_t) writeIndex_] = inputRight;
    writeIndex_ = (writeIndex_ + 1) % size_;

    StereoSample out;
    out.left = inputLeft * (1.0f - mix_) + (inputLeft + wetLeft) * mix_ * 0.7f;
    out.right = inputRight * (1.0f - mix_) + (inputRight + wetRight) * mix_ * 0.7f;
    return out;
}

// ---------------------------------------------------------------------------
// Phaser
// ---------------------------------------------------------------------------

void PhaserEffect::prepare (double sampleRate)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    reset();
    setRateHz (0.25f);
    setDepth (0.6f);
}

void PhaserEffect::reset()
{
    phase_ = 0.0f;
    lastLeft_ = lastRight_ = 0.0f;
    for (int i = 0; i < 6; ++i)
    {
        stateLeft_[i] = 0.0f;
        stateRight_[i] = 0.0f;
    }
}

void PhaserEffect::setRateHz (float hz)
{
    increment_ = kTwoPi * clampf (hz, 0.01f, 8.0f) / (float) sampleRate_;
}

void PhaserEffect::setDepth (float depth01) { depth_ = clampf (depth01, 0.0f, 1.0f); }
void PhaserEffect::setFeedback (float feedback01) { feedback_ = clampf (feedback01, 0.0f, 0.95f); }
void PhaserEffect::setMix (float mix01) { mix_ = clampf (mix01, 0.0f, 1.0f); }

void PhaserEffect::updateCoefficients (float lfo)
{
    for (int i = 0; i < 6; ++i)
    {
        const auto offset = (float) i * 0.12f;
        const auto freq = 200.0f * std::pow (20.0f, (lfo + 0.5f) * depth_ + offset * 0.2f);
        const auto t = std::tan (kPi * clampf (freq, 80.0f, 5000.0f) / (float) sampleRate_);
        const auto coefficient = (t - 1.0f) / (t + 1.0f);
        coefLeft_[i] = coefficient;
        coefRight_[i] = coefficient;
    }
}

StereoSample PhaserEffect::process (float inputLeft, float inputRight) noexcept
{
    phase_ += increment_;
    if (phase_ > kTwoPi)
        phase_ -= kTwoPi;

    updateCoefficients (std::sin (phase_));

    auto left = inputLeft + lastLeft_ * feedback_;
    auto right = inputRight + lastRight_ * feedback_;

    for (int i = 0; i < 6; ++i)
    {
        const auto yl = coefLeft_[i] * left + stateLeft_[i];
        stateLeft_[i] = left - coefLeft_[i] * yl;
        left = yl;

        const auto yr = coefRight_[i] * right + stateRight_[i];
        stateRight_[i] = right - coefRight_[i] * yr;
        right = yr;
    }

    lastLeft_ = left;
    lastRight_ = right;

    StereoSample out;
    out.left = inputLeft * (1.0f - mix_ * 0.5f) + left * mix_ * 0.5f;
    out.right = inputRight * (1.0f - mix_ * 0.5f) + right * mix_ * 0.5f;
    return out;
}

// ---------------------------------------------------------------------------
// Convolution
// ---------------------------------------------------------------------------

void ConvolutionEffect::prepare (double sampleRate, int seed)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    const auto taps = 384;
    const auto noiseSeed = (uint32_t) (seed * 2654435761u + 0x9e3779b9u);

    size_ = 1024;
    writeIndex_ = 0;

    for (int channel = 0; channel < 2; ++channel)
    {
        impulse_[(size_t) channel].assign ((size_t) taps, 0.0f);
        buffer_[(size_t) channel].assign ((size_t) size_, 0.0f);
        uint32_t state = noiseSeed + (uint32_t) (channel * 7919);

        for (int i = 0; i < taps; ++i)
        {
            state = state * 1664525u + 1013904223u;
            const auto noise = (float) ((int32_t) state) / 2147483648.0f;
            impulse_[(size_t) channel][(size_t) i] =
                noise * std::exp (-(float) i / (float) taps * 3.2f);
        }

        impulse_[(size_t) channel][0] += 0.7f;

        float energy = 0.0f;
        for (auto v : impulse_[(size_t) channel])
            energy += v * v;

        const auto norm = 1.0f / std::sqrt (std::max (1.0e-9f, energy));
        for (auto& v : impulse_[(size_t) channel])
            v *= norm;
    }
    for (int model = 0; model < 4; ++model)
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 384; ++i)
            {
                const auto base = impulse_[(size_t) ch][(size_t) i];
                library_[(size_t) model][(size_t) ch][(size_t) i] = model == 0 ? base
                    : (model == 1 ? base * std::cos (i * 0.13f) : (model == 2
                       ? std::sin (i * (ch == 0 ? .21f : .23f)) * std::exp (-i * .012f) * .08f
                       : (i % 41 == 0 ? .35f * std::exp (-i * .009f) : 0.0f)));
            }

}

void ConvolutionEffect::setControls (int model, float length, float tuning, float width) noexcept
{
    model_ = std::clamp (model, 0, 3);
    length_ = clampf (length, 0.05f, 1.0f);
    tuning_ = clampf (tuning, 0.5f, 2.0f);
    width_ = clampf (width, 0.0f, 2.0f);
}

void ConvolutionEffect::reset()
{
    for (auto& b : buffer_)
        std::fill (b.begin(), b.end(), 0.0f);
    writeIndex_ = 0;
}

void ConvolutionEffect::setMix (float mix01)
{
    mix_ = std::max (0.0f, std::min (1.0f, mix01));
}

StereoSample ConvolutionEffect::process (float inputLeft, float inputRight) noexcept
{
    buffer_[0][(size_t) writeIndex_] = inputLeft;
    buffer_[1][(size_t) writeIndex_] = inputRight;

    float wetLeft = 0.0f;
    float wetRight = 0.0f;
    const auto taps = std::min (768, (int) (384 * length_ / tuning_));
    if (mix_ <= 0.0f) { writeIndex_ = (writeIndex_ + 1) % size_; return { inputLeft, inputRight }; }

    for (int k = 0; k < taps; ++k)
    {
        auto index = writeIndex_ - k;
        if (index < 0)
            index += size_;

        const auto irPosition = std::min (383.0f, k * tuning_);
        const auto lo = (int) irPosition, hi = std::min (383, lo + 1);
        const auto f = irPosition - lo;
        const auto& ir = library_[(size_t) model_];
        wetLeft += buffer_[0][(size_t) index] * lerpf (ir[0][(size_t) lo], ir[0][(size_t) hi], f);
        wetRight += buffer_[1][(size_t) index] * lerpf (ir[1][(size_t) lo], ir[1][(size_t) hi], f);
    }

    writeIndex_ = (writeIndex_ + 1) % size_;

    if (width_ != 1.0f)
    {
        const auto mid = (wetLeft + wetRight) * 0.5f;
        const auto side = (wetLeft - wetRight) * 0.5f * width_;
        wetLeft = mid + side; wetRight = mid - side;
    }
    StereoSample out;
    out.left = inputLeft * (1.0f - mix_) + wetLeft * mix_;
    out.right = inputRight * (1.0f - mix_) + wetRight * mix_;
    return out;
}

// ---------------------------------------------------------------------------
// Reverb
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void ReverbEffect::prepare (double sampleRate)
{
    sampleRate_ = std::max (8000.0, sampleRate);

    const double lengthsMs[4] = { 23.7, 29.3, 34.9, 41.1 };
    for (int i = 0; i < 4; ++i)
    {
        auto& line = lines_[(size_t) i];
        const auto length = (int) (sampleRate_ * lengthsMs[i] * 0.001) + 2;
        line.buffer.assign ((size_t) length, 0.0f);
        line.writeIndex = 0;
        line.damping = 0.0f;
    }

    setSize (0.6f);
    setDamping (0.3f);
    setMix (0.25f);
}

void ReverbEffect::reset()
{
    for (auto& line : lines_)
    {
        std::fill (line.buffer.begin(), line.buffer.end(), 0.0f);
        line.writeIndex = 0;
        line.damping = 0.0f;
    }
}

void ReverbEffect::setSize (float size01)
{
    const auto size = clampf (size01, 0.0f, 1.0f);
    decay_ = 0.72f + 0.26f * size;
}

void ReverbEffect::setDamping (float damping01)
{
    damp_ = clampf (damping01, 0.0f, 1.0f) * 0.85f;
}

void ReverbEffect::setMix (float mix01)
{
    mix_ = clampf (mix01, 0.0f, 1.0f);
}

int ReverbEffect::read (const DelayLine& line) const noexcept
{
    const auto length = (int) line.buffer.size();
    const auto index = (line.writeIndex + length) % length;
    return index;
}

StereoSample ReverbEffect::process (float inputLeft, float inputRight) noexcept
{
    const auto input = (inputLeft + inputRight) * 0.5f;

    float d[4] {};
    for (int i = 0; i < 4; ++i)
    {
        auto& line = lines_[(size_t) i];
        const auto index = read (line);
        const auto value = line.buffer[(size_t) index];
        line.damping += (value - line.damping) * (0.05f + damp_ * 0.35f);
        d[i] = line.damping;
    }

    // Normalised Hadamard feedback, with the input injected into all lines.
    const auto y0 = (d[0] + d[1] + d[2] + d[3]) * 0.5f;
    const auto y1 = (d[0] - d[1] + d[2] - d[3]) * 0.5f;
    const auto y2 = (d[0] + d[1] - d[2] - d[3]) * 0.5f;
    const auto y3 = (d[0] - d[1] - d[2] + d[3]) * 0.5f;

    const float feedback[4] = { y0, y1, y2, y3 };
    for (int i = 0; i < 4; ++i)
    {
        auto& line = lines_[(size_t) i];
        line.buffer[(size_t) line.writeIndex] = input * 0.35f + feedback[i] * decay_;
        line.writeIndex = (line.writeIndex + 1) % (int) line.buffer.size();
    }

    StereoSample out;
    const auto wetLeft = (d[0] + d[2]) * 0.6f;
    const auto wetRight = (d[1] + d[3]) * 0.6f;
    out.left = inputLeft * (1.0f - mix_) + wetLeft * mix_;
    out.right = inputRight * (1.0f - mix_) + wetRight * mix_;
    return out;
}

// ---------------------------------------------------------------------------
// Distortion
// ---------------------------------------------------------------------------

float softClip (float input) noexcept
{
    return std::tanh (input);
}

float applyDistortion (float input, DistortionType type, float drive) noexcept
{
    const auto d = clampf (drive, 0.1f, 40.0f);
    const auto x = input * d;

    switch (type)
    {
        case DistortionType::Soft:
            return std::tanh (x) * 0.8f;

        case DistortionType::Hard:
            return clampf (x, -1.0f, 1.0f) * 0.8f;

        case DistortionType::Fold:
        {
            auto folded = std::fmod (x + 1.0f, 4.0f);
            if (folded < 0.0f)
                folded += 4.0f;
            return (std::abs (folded - 2.0f) - 1.0f) * 0.8f;
        }

        case DistortionType::Tube:
            return (x >= 0.0f ? std::tanh (x) : std::tanh (x * 0.7f) * 0.8f) * 0.85f;
    }

    return input;
}

} // namespace triad
