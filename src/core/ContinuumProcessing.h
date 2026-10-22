#pragma once
#include "core/Dsp.h"
#include <algorithm>
#include <cmath>

namespace triad
{
/** Stereo-linked compressor and complementary tilt shelves. Defaults bypass. */
class ContinuumMasterProcessor
{
public:
    void prepare (double rate) { rate_ = std::max (8000.0, rate); reset(); }
    void reset() { envelope_ = 0; lowL_ = lowR_ = 0; }
    void configure (bool enabled, float thresholdDb, float ratio, float attack, float release,
                    float makeupDb, float tiltDb, float pivotHz)
    {
        enabled_ = enabled; threshold_ = thresholdDb; ratio_ = std::max (1.0f, ratio);
        attack_ = std::exp (-1.0f / ((float) rate_ * std::max (.0001f, attack)));
        release_ = std::exp (-1.0f / ((float) rate_ * std::max (.001f, release)));
        makeup_ = std::pow (10.0f, makeupDb / 20.0f);
        tilt_ = tiltDb;
        lowGain_ = std::pow (10.0f, -tiltDb / 40.0f); highGain_ = std::pow (10.0f, tiltDb / 40.0f);
        pole_ = 1.0f - std::exp (-2.0f * kPi * std::clamp (pivotHz, 20.0f, (float) rate_ * .45f) / (float) rate_);
    }
    StereoSample processCompressor (float l, float r)
    {
        const auto level = std::max (std::abs (l), std::abs (r));
        const auto coefficient = level > envelope_ ? attack_ : release_;
        envelope_ = level + coefficient * (envelope_ - level);
        if (enabled_)
        {
            const auto db = 20.0f * std::log10 (std::max (envelope_, 1.0e-9f));
            const auto reduction = std::max (0.0f, db - threshold_) * (1.0f - 1.0f / ratio_);
            const auto gain = std::pow (10.0f, -reduction / 20.0f) * makeup_;
            l *= gain; r *= gain;
        }
        return { l, r };
    }
    StereoSample processTilt (float l, float r)
    {
        lowL_ += pole_ * (l - lowL_); lowR_ += pole_ * (r - lowR_);
        if (std::abs (tilt_) > 1.0e-6f)
        {
            l = lowL_ * lowGain_ + (l - lowL_) * highGain_;
            r = lowR_ * lowGain_ + (r - lowR_) * highGain_;
        }
        return { l, r };
    }
    StereoSample process (float l, float r)
    {
        const auto c = processCompressor (l, r);
        return processTilt (c.left, c.right);
    }
private:
    double rate_ = 48000;
    float envelope_ = 0, lowL_ = 0, lowR_ = 0, threshold_ = -18, ratio_ = 4;
    float attack_ = .99f, release_ = .999f, makeup_ = 1, lowGain_ = 1, highGain_ = 1, pole_ = .1f, tilt_ = 0;
    bool enabled_ = false;
};
} // namespace triad
