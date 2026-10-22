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

PatternSequencer::PatternSequencer()
{
    for (int i = 0; i < maxSteps; ++i)
    {
        stepActive_[(size_t) i].store (i % 2 == 0);
        stepNote_[(size_t) i].store (48 + (i % 4) * 3);
        stepVelocity_[(size_t) i].store (i % 4 == 0 ? 110 : 84);
    }
}

void PatternSequencer::prepare (double sampleRate)
{
    sampleRate_ = std::max (8000.0, sampleRate);
    reset();
}

void PatternSequencer::reset()
{
    heldCount_ = 0;
    orderIndex_ = 0;
    arpDescending_ = false;
    sampleAccumulator_ = 0.0;
    gateCountdown_ = 0;
    activeNote_ = -1;
    patternIndex_ = 0;
    lastStep_ = -1;
    activeStep_.store (-1);
    currentNote_.store (-1);
}

void PatternSequencer::setStepActive (int step, bool active)
{
    if (step >= 0 && step < maxSteps)
        stepActive_[(size_t) step].store (active);
}

void PatternSequencer::setStepNote (int step, int note)
{
    if (step >= 0 && step < maxSteps)
        stepNote_[(size_t) step].store (std::clamp (note, 0, 127));
}

void PatternSequencer::setStepVelocity (int step, int velocity)
{
    if (step >= 0 && step < maxSteps)
        stepVelocity_[(size_t) step].store (std::clamp (velocity, 1, 127));
}

bool PatternSequencer::isStepActive (int step) const
{
    return step >= 0 && step < maxSteps && stepActive_[(size_t) step].load();
}

int PatternSequencer::stepNote (int step) const
{
    return step >= 0 && step < maxSteps ? stepNote_[(size_t) step].load() : 60;
}

int PatternSequencer::stepVelocity (int step) const
{
    return step >= 0 && step < maxSteps ? stepVelocity_[(size_t) step].load() : 100;
}

void PatternSequencer::noteOn (int note, float velocity)
{
    for (int i = 0; i < heldCount_; ++i)
        if (heldNotes_[(size_t) i] == note)
            return;

    if (heldCount_ >= maxHeldNotes)
    {
        for (int i = 1; i < heldCount_; ++i)
        {
            heldNotes_[(size_t) (i - 1)] = heldNotes_[(size_t) i];
            heldVelocity_[(size_t) (i - 1)] = heldVelocity_[(size_t) i];
        }
        --heldCount_;
    }

    heldNotes_[(size_t) heldCount_] = note;
    heldVelocity_[(size_t) heldCount_] = std::clamp (velocity, 0.01f, 1.0f);
    ++heldCount_;
}

void PatternSequencer::noteOff (int note)
{
    if (hold_.load())
        return;

    for (int i = 0; i < heldCount_; ++i)
    {
        if (heldNotes_[(size_t) i] == note)
        {
            for (int j = i; j < heldCount_ - 1; ++j)
            {
                heldNotes_[(size_t) j] = heldNotes_[(size_t) (j + 1)];
                heldVelocity_[(size_t) j] = heldVelocity_[(size_t) (j + 1)];
            }
            --heldCount_;
            return;
        }
    }
}

void PatternSequencer::allNotesOff()
{
    heldCount_ = 0;
    gateCountdown_ = 1;
    activeStep_.store (-1);
}

void PatternSequencer::setHoldActive (bool active)
{
    if (! active)
        heldCount_ = 0;
}

void PatternSequencer::clearHeld()
{
    heldCount_ = 0;
}

int PatternSequencer::pickArpNote()
{
    if (heldCount_ == 0)
        return -1;

    std::array<int, maxHeldNotes> notes {};
    for (int i = 0; i < heldCount_; ++i)
        notes[(size_t) i] = heldNotes_[(size_t) i];

    std::sort (notes.begin(), notes.begin() + heldCount_);

    const auto mode = (Mode) std::clamp (mode_.load(), 0, 6);
    const auto octaveCount = std::clamp (octaves_.load(), 1, 4);
    const auto noteCount = heldCount_;

    int index = 0;

    switch (mode)
    {
        case Mode::Down:
            index = (noteCount - 1) - (orderIndex_ % noteCount);
            break;

        case Mode::UpDown:
        {
            const auto cycle = std::max (1, noteCount * 2 - 2);
            const auto position = orderIndex_ % cycle;
            index = position < noteCount ? position : cycle - position;
            break;
        }

        case Mode::Random:
            randomState_ ^= randomState_ << 13;
            randomState_ ^= randomState_ >> 17;
            randomState_ ^= randomState_ << 5;
            index = (int) (randomState_ % (uint32_t) noteCount);
            break;

        case Mode::Order:
            index = orderIndex_ % noteCount;
            break;

        case Mode::Up:
        default:
            index = orderIndex_ % noteCount;
            break;
    }

    const auto octave = (orderIndex_ / noteCount) % octaveCount;
    ++orderIndex_;

    return std::clamp (notes[(size_t) index] + 12 * octave, 0, 127);
}

int PatternSequencer::process (int numSamples, Event* events, int maxEventCount)
{
    eventsWritten_ = 0;
    if (events == nullptr || maxEventCount <= 0 || numSamples <= 0)
        return 0;

    const auto mode = (Mode) std::clamp (mode_.load(), 0, 6);

    if (mode == Mode::Off || ! playing_.load())
    {
        if (activeNote_ >= 0 && eventsWritten_ < maxEventCount)
        {
            events[eventsWritten_++] = { 0, false, activeNote_, 0.0f };
            activeNote_ = -1;
            currentNote_.store (-1);
            activeStep_.store (-1);
        }

        sampleAccumulator_ = 0.0;
        return eventsWritten_;
    }

    const auto bpm = std::max (20.0, tempo_.load());
    static const double stepsPerBeat[6] = { 1.0, 2.0, 3.0, 4.0, 6.0, 8.0 };
    const auto rate = std::clamp (rateIndex_.load(), 0, 5);
    const auto beatsPerStep = 1.0 / stepsPerBeat[rate];
    samplesPerStep_ = std::max (4.0, (60.0 / bpm) * sampleRate_ * beatsPerStep);

    const auto gate = std::clamp (gate_.load(), 0.05f, 1.0f);
    gateSamples_ = std::max (1.0, samplesPerStep_ * (double) gate);

    for (int sample = 0; sample < numSamples; ++sample)
    {
        if (gateCountdown_ > 0)
        {
            --gateCountdown_;
            if (gateCountdown_ == 0 && activeNote_ >= 0 && eventsWritten_ < maxEventCount)
            {
                events[eventsWritten_++] = { sample, false, activeNote_, 0.0f };
                activeNote_ = -1;
                currentNote_.store (-1);
            }
        }

        if (sampleAccumulator_ <= 0.0)
        {
            int note = -1;
            float velocity = 0.8f;

            if (mode == Mode::Step)
            {
                const auto count = std::clamp (stepCount_.load(), 1, maxSteps);
                const auto step = patternIndex_ % count;
                patternIndex_ = (patternIndex_ + 1) % count;
                lastStep_ = step;
                activeStep_.store (step);

                if (stepActive_[(size_t) step].load())
                {
                    const auto root = heldCount_ > 0 ? heldNotes_[0] : 60;
                    note = std::clamp (stepNote_[(size_t) step].load() + (root - 60), 0, 127);
                    velocity = (float) stepVelocity_[(size_t) step].load() / 127.0f;
                }
            }
            else
            {
                note = pickArpNote();
                const auto step = (orderIndex_ / std::max (1, heldCount_)) % 16;
                lastStep_ = step;
                activeStep_.store (step);

                if (note >= 0)
                {
                    velocity = heldCount_ > 0 ? heldVelocity_[0] : 0.8f;
                }
            }

            if (note >= 0)
            {
                if (activeNote_ >= 0 && eventsWritten_ < maxEventCount)
                {
                    events[eventsWritten_++] = { sample, false, activeNote_, 0.0f };
                    activeNote_ = -1;
                }

                if (eventsWritten_ < maxEventCount)
                {
                    events[eventsWritten_++] = { sample, true, note, velocity };
                    activeNote_ = note;
                    currentNote_.store (note);
                    gateCountdown_ = (int) gateSamples_;
                }
            }

            const auto swing = std::clamp (swing_.load(), 0.0f, 0.6f);
            const auto swingScale = (patternIndex_ % 2 == 0) ? (1.0 + swing) : (1.0 - swing);
            sampleAccumulator_ += samplesPerStep_ * swingScale;
        }

        sampleAccumulator_ -= 1.0;
    }

    return eventsWritten_;
}

} // namespace triad
