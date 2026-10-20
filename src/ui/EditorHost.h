#pragma once

#include "core/Engine.h"
#include "core/Sequencer.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <utility>
#include <vector>

namespace triad
{

struct PatchCable
{
    int from = -1;
    int to = -1;
};

/** The narrow interface the editor needs from the plugin wrapper. */
class EditorHost
{
public:
    virtual ~EditorHost() = default;

    virtual juce::AudioProcessorValueTreeState& apvts() = 0;
    virtual const ModelDefinition& definition() const = 0;
    virtual SynthEngine& engine() = 0;
    virtual juce::MidiKeyboardState& keyboardState() = 0;
    virtual PatternSequencer& sequencer() = 0;

    virtual juce::String instrumentName() const = 0;

    virtual int selectedPart() const { return 1; }
    virtual void setSelectedPart (int) {}
    virtual bool hasParts() const { return false; }

    virtual float uiPitchBend() const { return 0.0f; }
    virtual float uiModWheel() const { return 0.0f; }
    virtual void setUiPitchBend (float) {}
    virtual void setUiModWheel (float) {}

    virtual void captureMorphB() {}
    virtual void swapMorphB() {}
    virtual bool hasMorph() const { return false; }

    virtual std::vector<PatchCable> patchCables() const { return {}; }
    virtual void setPatchCable (int, int, bool) {}
    virtual void clearPatchCables() {}
    virtual bool hasPatching() const { return false; }

    virtual int factoryPresetCount() const { return 0; }
    virtual juce::String factoryPresetName (int) const { return {}; }
    virtual juce::String factoryPresetCategory (int) const { return {}; }
    virtual void loadFactoryPreset (int) {}
    virtual void initPatch() {}
    virtual void savePreset() {}
    virtual void loadPreset() {}
    virtual juce::String currentPresetName() const { return "INIT"; }

    // CONTINUUM V continuous surface ---------------------------------------
    virtual bool isContinuum() const { return false; }
    virtual void continuumNoteOn (int, float, float, float) {}
    virtual void continuumNoteOff (int) {}
    virtual void continuumSetExpression (int, float, float, float) {}
    virtual float continuumExpression (int, int) const { return 0.0f; }
};

} // namespace triad
