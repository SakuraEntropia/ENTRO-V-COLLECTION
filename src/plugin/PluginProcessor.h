#pragma once

#include "core/Engine.h"
#include "core/Sequencer.h"
#include "engines/Engines.h"
#include "plugin/Presets.h"
#include "ui/EditorHost.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>

#include <array>
#include <atomic>
#include <memory>
#include <vector>

namespace triad
{

class TriadAudioProcessor final : public juce::AudioProcessor,
                                  public EditorHost
{
public:
    TriadAudioProcessor();
    ~TriadAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override;
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // EditorHost -----------------------------------------------------------
    juce::AudioProcessorValueTreeState& apvts() override { return apvts_; }
    const ModelDefinition& definition() const override { return engine_->definition(); }
    SynthEngine& engine() override { return *engine_; }
    juce::MidiKeyboardState& keyboardState() override { return keyboardState_; }
    PatternSequencer& sequencer() override { return sequencer_; }
    juce::String instrumentName() const override { return definition().displayName; }

    int selectedPart() const override { return selectedPart_.load(); }
    void setSelectedPart (int part) override;
    bool hasParts() const override;

    float uiPitchBend() const override { return uiPitchBend_.load(); }
    float uiModWheel() const override { return uiModWheel_.load(); }
    void setUiPitchBend (float value) override { uiPitchBend_.store (value); }
    void setUiModWheel (float value) override { uiModWheel_.store (value); }

    void captureMorphB() override;
    void swapMorphB() override;
    bool hasMorph() const override;

    std::vector<PatchCable> patchCables() const override;
    void setPatchCable (int from, int to, bool connected) override;
    void clearPatchCables() override;
    bool hasPatching() const override;

    int factoryPresetCount() const override;
    juce::String factoryPresetName (int index) const override;
    juce::String factoryPresetCategory (int index) const override;
    void loadFactoryPreset (int index) override;
    void initPatch() override;
    void savePreset() override;
    void loadPreset() override;
    juce::String currentPresetName() const override;

private:
    struct ScheduledEvent
    {
        int offset = 0;
        MidiEvent event;
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout (const ModelDefinition&);
    static bool toMidiEvent (const juce::MidiMessage& message, MidiEvent& out);

    void readParameters();
    void updateSequencerFromParameters (const TransportInfo& transport);
    void applyPresetValues (const std::vector<std::pair<juce::String, float>>& values,
                            const juce::String& presetName, bool resetFirst);
    void setParameterNatural (const juce::String& id, float value);
    int findParameterBySuffix (const juce::String& suffix) const;
    int findMorphParameter() const;
    void addScheduledEvent (int offset, const MidiEvent& event);
    void encodeState (juce::ValueTree& state);
    void decodeState (const juce::ValueTree& state);
    void savePresetFile (const juce::File& file);
    void loadPresetFile (const juce::File& file);

    InstrumentKind kind_;
    std::unique_ptr<SynthEngine> engine_;
    juce::AudioProcessorValueTreeState apvts_;

    std::vector<std::atomic<float>*> parameterPointers_;
    std::vector<float> naturalValues_;
    std::unique_ptr<std::atomic<float>[]> morphValues_;
    std::vector<float> morphScratch_;
    int morphParameterIndex_ = -1;
    int selectedPartIndex_ = -1;

    juce::MidiKeyboardState keyboardState_;
    PatternSequencer sequencer_;
    std::vector<FactoryPreset> factoryPresets_;

    std::array<ScheduledEvent, 512> scheduled_ {};
    int scheduledCount_ = 0;

    std::vector<float> dummyRight_;
    float lastPitchBendSent_ = 0.0f;
    float lastModWheelSent_ = -1.0f;
    bool lastSequencerEnabled_ = false;

    int arpEnabledIndex_ = -1;
    int arpModeIndex_ = -1;
    int arpRateIndex_ = -1;
    int arpOctavesIndex_ = -1;
    int arpHoldIndex_ = -1;
    int arpLengthIndex_ = -1;
    int arpSwingIndex_ = -1;
    bool lastArpHold_ = false;

    mutable juce::CriticalSection patchLock_;
    std::vector<PatchCable> patchCables_;

    std::atomic<int> selectedPart_ { 1 };
    std::atomic<float> uiPitchBend_ { 0.0f };
    std::atomic<float> uiModWheel_ { 0.0f };

    juce::String currentPresetName_ { "INIT" };
    int currentProgram_ = 0;
    std::shared_ptr<std::atomic<bool>> aliveFlag_;
    std::unique_ptr<juce::FileChooser> fileChooser_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriadAudioProcessor)
};

} // namespace triad
