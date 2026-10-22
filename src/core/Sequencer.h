#pragma once

#include <array>
#include <atomic>
#include <cstdint>

namespace triad
{

/** Four-interval tap average; a pause starts a fresh sequence of taps. */
class TapTempo
{
public:
    double tap (double milliseconds);
private:
    std::array<double, 4> intervals_ {};
    double lastTap_ = -1.0;
    int count_ = 0, next_ = 0;
};

/** Block-driven arpeggiator and step sequencer.
    The audio callback advances it sample by sample and emits note events with
    sample offsets. Pattern edits are lock-free atomics written from the UI. */
class PatternSequencer
{
public:
    static constexpr int maxSteps = 16;
    static constexpr int maxHeldNotes = 16;
    static constexpr int maxEvents = 64;

    enum class Mode
    {
        Off = 0,
        Up,
        Down,
        UpDown,
        Random,
        Order,
        Step
    };

    struct Event
    {
        int sampleOffset = 0;
        bool noteOn = false;
        int note = 60;
        float velocity = 0.8f;
    };

    PatternSequencer();

    void prepare (double sampleRate);
    void reset();

    void setMode (int mode) { mode_.store (mode); }
    void setRateIndex (int index) { rateIndex_.store (index); }
    void setOctaves (int octaves) { octaves_.store (octaves); }
    void setHold (bool hold) { hold_.store (hold); }
    void setGate (float gate) { gate_.store (gate); }
    void setSwing (float swing) { swing_.store (swing); }
    void setStepCount (int steps) { stepCount_.store (steps); }
    void setTempo (double bpm) { tempo_.store (bpm); }
    void setPlaying (bool playing) { playing_.store (playing); }

    void setStepActive (int step, bool active);
    void setStepNote (int step, int note);
    void setStepVelocity (int step, int velocity);
    bool isStepActive (int step) const;
    int stepNote (int step) const;
    int stepVelocity (int step) const;

    void noteOn (int note, float velocity);
    void noteOff (int note);
    void allNotesOff();
    void setHoldActive (bool active);
    void clearHeld();

    int currentStep() const { return activeStep_.load(); }
    int activeNote() const { return activeNote_.load(); }

    /** Generates events for the next block. Returns the number written. */
    int process (int numSamples, Event* events, int maxEventCount);

private:
    int pickArpNote();

    double sampleRate_ = 48000.0;

    std::atomic<int> mode_ { 0 };
    std::atomic<int> rateIndex_ { 3 };
    std::atomic<int> octaves_ { 1 };
    std::atomic<bool> hold_ { false };
    std::atomic<float> gate_ { 0.55f };
    std::atomic<float> swing_ { 0.0f };
    std::atomic<int> stepCount_ { 16 };
    std::atomic<double> tempo_ { 120.0 };
    std::atomic<bool> playing_ { false };

    std::atomic<bool> stepActive_[maxSteps];
    std::atomic<int> stepNote_[maxSteps];
    std::atomic<int> stepVelocity_[maxSteps];

    std::array<int, maxHeldNotes> heldNotes_ {};
    std::array<float, maxHeldNotes> heldVelocity_ {};
    int heldCount_ = 0;
    int orderIndex_ = 0;
    bool arpDescending_ = false;
    uint32_t randomState_ = 0x8f31a2b7u;

    double sampleAccumulator_ = 0.0;
    double samplesPerStep_ = 6000.0;
    double gateSamples_ = 3000.0;
    int gateCountdown_ = 0;
    std::atomic<int> activeNote_ { -1 };
    int patternIndex_ = 0;
    int lastStep_ = -1;
    std::atomic<int> activeStep_ { -1 };
    std::atomic<int> currentNote_ { -1 };

    int eventsWritten_ = 0;
};

} // namespace triad
