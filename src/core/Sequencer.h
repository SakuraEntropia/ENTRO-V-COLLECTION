#pragma once
#include <array>
#include <atomic>
#include <cstdint>

namespace triad {
class TapTempo {
public: double tap (double milliseconds);
private:
    std::array<double, 4> intervals_ {};
    double lastTap_ = -1.0;
    int count_ = 0, next_ = 0;
};

/** Bounded polyphonic pattern/arp engine. UI edits are atomic; MIDI and process
    belong to the audio thread. A tie connects this step to the following step. */
class PatternSequencer {
public:
    static constexpr int maxSteps = 256, maxPatterns = 12, maxPolyphony = 6;
    static constexpr int maxHeldNotes = 32, maxEvents = 512;
    enum class Mode { Off = 0, Up, Down, UpDown, Random, Order, Step };
    struct Event { int sampleOffset = 0; bool noteOn = false; int note = 60; float velocity = 0.8f; };
    PatternSequencer();
    void prepare (double sampleRate);
    void reset();
    void setLimits (int steps, int notes, int patterns);
    int maximumSteps() const { return limitSteps_; }
    int polyphony() const { return limitNotes_; }
    int patternCount() const { return limitPatterns_; }
    void selectPattern (int pattern);
    int selectedPattern() const { return selectedPattern_.load(); }
    void setMode (int mode) { mode_.store (mode); }
    void setRateIndex (int index) { rateIndex_.store (index); }
    void setOctaves (int octaves) { octaves_.store (octaves); }
    void setHold (bool hold) { hold_.store (hold); }
    void setGate (float gate) { gate_.store (gate); }
    float gate() const { return gate_.load(); }
    void setSwing (float swing) { swing_.store (swing); }
    void setStepCount (int steps);
    int stepCount() const;
    void setPatternLength (int pattern, int steps);
    int patternLength (int pattern) const;
    void setTempo (double bpm) { tempo_.store (bpm); }
    void setPlaying (bool playing) { playing_.store (playing); }
    void setStepActive (int step, bool active);
    void setStepNote (int step, int note);
    void setStepVelocity (int step, int velocity);
    bool isStepActive (int step) const;
    int stepNote (int step) const;
    int stepVelocity (int step) const;
    void setPatternNote (int pattern, int step, int slot, int note); // -1 removes a note
    int patternNote (int pattern, int step, int slot) const;
    void setPatternActive (int pattern, int step, bool active);
    bool patternActive (int pattern, int step) const;
    void setPatternVelocity (int pattern, int step, int velocity);
    int patternVelocity (int pattern, int step) const;
    void setPatternTie (int pattern, int step, bool tie);
    bool patternTie (int pattern, int step) const;
    void clearPattern (int pattern);
    void noteOn (int note, float velocity);
    void noteOff (int note);
    void allNotesOff();
    void setHoldActive (bool active);
    void clearHeld();
    int currentStep() const { return activeStep_.load(); }
    int activeNote() const { return activeNote_.load(); }
    int process (int numSamples, Event* events, int maxEventCount);
private:
    struct Step {
        std::array<std::atomic<int>, maxPolyphony> notes;
        std::atomic<int> velocity { 100 };
        std::atomic<bool> active { false }, tie { false };
    };
    bool valid (int pattern, int step) const;
    int pickArpNote();
    double sampleRate_ = 48000.0;
    int limitSteps_ = 16, limitNotes_ = 1, limitPatterns_ = 1;
    std::array<std::array<Step, maxSteps>, maxPatterns> patterns_;
    std::array<std::atomic<int>, maxPatterns> lengths_;
    std::atomic<int> selectedPattern_ { 0 }, mode_ { 0 }, rateIndex_ { 3 }, octaves_ { 1 };
    std::atomic<bool> hold_ { false }, playing_ { false };
    std::atomic<float> gate_ { 0.55f }, swing_ { 0.0f };
    std::atomic<double> tempo_ { 120.0 };
    std::array<int, maxHeldNotes> heldNotes_ {};
    std::array<float, maxHeldNotes> heldVelocity_ {};
    int heldCount_ = 0, orderIndex_ = 0;
    uint32_t randomState_ = 0x8f31a2b7u;
    double sampleAccumulator_ = 0.0;
    int gateCountdown_ = 0, patternIndex_ = 0, lastPattern_ = -1;
    std::array<int, maxPolyphony> sounding_ {};
    int soundingCount_ = 0;
    bool previousTie_ = false;
    std::atomic<int> activeNote_ { -1 }, activeStep_ { -1 };
};
} // namespace triad
