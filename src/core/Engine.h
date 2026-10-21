#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace triad
{

inline constexpr float kPi = 3.14159265358979323846f;
inline constexpr float kTwoPi = 6.28318530717958647692f;

// ---------------------------------------------------------------------------
// Static model metadata
// ---------------------------------------------------------------------------

enum class ParamCurve
{
    Linear,
    Log,
    Discrete
};

struct ParamSpec
{
    std::string id;
    std::string name;
    std::string group;
    std::string unit;
    float min = 0.0f;
    float max = 1.0f;
    float def = 0.0f;
    ParamCurve curve = ParamCurve::Linear;
    std::vector<std::string> choices;

    /** A two-state switch expressed as a choice parameter. */
    bool isToggle() const
    {
        return curve == ParamCurve::Discrete && choices.size() == 2
            && choices[0] == "Off" && choices[1] == "On";
    }
};

struct PatchPointSpec
{
    std::string id;
    std::string name;
    std::string group;
    bool isOutput = false;
    bool audioRate = false;
};

struct ModSourceSpec
{
    std::string id;
    std::string name;
};

struct ModDestSpec
{
    std::string id;
    std::string name;
};

/** Immutable, per-instrument parameter and patch metadata. */
struct ModelDefinition
{
    std::string id;
    std::string displayName;
    std::string manufacturer = "SakuraEntropia";
    std::string version = "0.1.0";
    std::vector<ParamSpec> params;
    std::vector<PatchPointSpec> patchPoints;
    std::vector<ModSourceSpec> modSources;
    std::vector<ModDestSpec> modDestinations;
    std::vector<std::string> groups;

    int indexOf (std::string_view paramId) const
    {
        for (size_t i = 0; i < params.size(); ++i)
            if (params[i].id == paramId)
                return static_cast<int> (i);

        return -1;
    }

    const ParamSpec* find (std::string_view paramId) const
    {
        const auto index = indexOf (paramId);
        return index >= 0 ? &params[(size_t) index] : nullptr;
    }

    int patchIndexOf (std::string_view patchId) const
    {
        for (size_t i = 0; i < patchPoints.size(); ++i)
            if (patchPoints[i].id == patchId)
                return static_cast<int> (i);

        return -1;
    }

    int modSourceIndex (std::string_view sourceId) const
    {
        for (size_t i = 0; i < modSources.size(); ++i)
            if (modSources[i].id == sourceId)
                return static_cast<int> (i);

        return -1;
    }

    int modDestIndex (std::string_view destId) const
    {
        for (size_t i = 0; i < modDestinations.size(); ++i)
            if (modDestinations[i].id == destId)
                return static_cast<int> (i);

        return -1;
    }

    int addParam (ParamSpec spec)
    {
        if (std::find (groups.begin(), groups.end(), spec.group) == groups.end())
            groups.push_back (spec.group);

        params.push_back (std::move (spec));
        return static_cast<int> (params.size()) - 1;
    }

    int addPatch (PatchPointSpec spec)
    {
        patchPoints.push_back (std::move (spec));
        return static_cast<int> (patchPoints.size()) - 1;
    }
};

// ---------------------------------------------------------------------------
// Audio / MIDI contracts
// ---------------------------------------------------------------------------

struct StereoSample
{
    float left = 0.0f;
    float right = 0.0f;
};

enum class MidiEventType
{
    NoteOn,
    NoteOff,
    AllNotesOff,
    AllSoundOff,
    PitchBend,
    ModWheel,
    ChannelPressure,
    SustainPedal,
    ControlChange
};

struct MidiEvent
{
    MidiEventType type = MidiEventType::NoteOn;
    int channel = 0;
    int note = 60;
    int controller = 0;
    /** Normalised meaning depends on the type: velocity 0..1, bend -1..1, pressure 0..1. */
    float value = 0.0f;
};

struct TransportInfo
{
    double bpm = 120.0;
    double ppqPosition = 0.0;
    bool isPlaying = false;
    bool isRecording = false;
};

/** Framework-independent audio engine contract.
    prepare() may allocate. render() and processMidi() must not allocate, open files or block. */
class SynthEngine
{
public:
    virtual ~SynthEngine() = default;

    virtual const ModelDefinition& definition() const = 0;

    virtual void prepare (double sampleRate, int maxBlockSize) = 0;
    virtual void reset() = 0;

    /** Natural-unit parameter values in definition().params order. */
    virtual void setParameters (const float* values, int count) = 0;

    /** PolyBrute-style second parameter snapshot for A/B morphing. */
    virtual void setMorphSnapshot (const float*, int) {}
    virtual void setMorphAmount (float) {}

    virtual void setTransport (const TransportInfo&) {}
    virtual void processMidi (const MidiEvent&) = 0;
    virtual void render (float* left, float* right, int numSamples) = 0;

    virtual int activeVoiceCount() const = 0;
    virtual float outputLevel() const { return 0.0f; }

    virtual bool supportsPatching() const { return false; }
    virtual void setPatchCable (int /*fromOutput*/, int /*toInput*/, bool /*connected*/) {}
    virtual void clearPatchCables() {}

    // EaganMatrix-style routing matrix (only implemented by CONTINUUM V).
    virtual int matrixRows() const { return 0; }
    virtual int matrixColumns() const { return 0; }
    virtual std::string matrixSourceName (int) const { return {}; }
    virtual std::string matrixDestinationName (int) const { return {}; }
    virtual int matrixFormulaCount() const { return 0; }
    virtual std::string matrixFormulaName (int) const { return {}; }
    virtual void setMatrixCell (int, int, int, float) {}
    virtual int matrixCellFormula (int, int) const { return 0; }
    virtual float matrixCellAmount (int, int) const { return 0.0f; }
    virtual void resetMatrix() {}
    virtual void setUserFormula (const std::string&) {}

    /** Per-note continuous expression for the on-screen Continuum surface. */
    virtual void setNoteExpression (int /*note*/, float /*pitchSemitones*/,
                                    float /*timbre01*/, float /*pressure01*/) {}
};

enum class InstrumentKind
{
    Matriarch = 0,
    PolyBrute = 1,
    MoogOne = 2,
    Continuum = 3
};

/** Small deterministic RNG shared by analog-drift and noise code. */
class Rng
{
public:
    explicit Rng (uint32_t seed = 0x1234567u) : state_ (seed | 1u) {}

    uint32_t nextUInt()
    {
        state_ ^= state_ << 13;
        state_ ^= state_ >> 17;
        state_ ^= state_ << 5;
        return state_;
    }

    /** Uniform in [-1, 1]. */
    float nextBipolar()
    {
        return (float) ((int32_t) (nextUInt() >> 8) - 8388608) / 8388607.0f;
    }

    float nextUnipolar()
    {
        return (float) (nextUInt() >> 8) / 16777215.0f;
    }

    void seed (uint32_t value) { state_ = value | 1u; }

private:
    uint32_t state_;
};

inline float clampf (float value, float low, float high)
{
    return value < low ? low : (value > high ? high : value);
}

inline double clampd (double value, double low, double high)
{
    return value < low ? low : (value > high ? high : value);
}

inline float lerpf (float a, float b, float t)
{
    return a + (b - a) * t;
}

inline float noteToHz (float midiNote)
{
    return 440.0f * std::pow (2.0f, (midiNote - 69.0f) / 12.0f);
}

inline float semitonesToRatio (float semitones)
{
    return std::pow (2.0f, semitones / 12.0f);
}

} // namespace triad
