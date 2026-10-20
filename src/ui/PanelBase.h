#pragma once

#include "ui/EditorHost.h"
#include "ui/LookAndFeel.h"
#include "ui/Widgets.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>
#include <vector>

namespace triad::ui
{

/** Shared physical-panel scaffolding: module frames, hardware controls and patching. */
class PanelBase : public juce::Component
{
public:
    PanelBase (EditorHost& host, juce::String title, int designWidth, int designHeight,
               juce::Colour accent);
    ~PanelBase() override;

    EditorHost& host() { return host_; }
    const ModelDefinition& definition() const { return host_.definition(); }
    juce::AudioProcessorValueTreeState& apvts() { return host_.apvts(); }
    juce::Colour accent() const { return accent_; }

    int designWidth() const { return designWidth_; }
    int designHeight() const { return designHeight_; }

    void setBranding (juce::String mainText, juce::String subText);

    enum class ModuleTone { Dark, Light };

    void addModule (const juce::String& title, juce::Rectangle<int> area,
                    ModuleTone tone = ModuleTone::Dark);
    void addModule (const juce::String& title, const juce::String& subtitle,
                    juce::Rectangle<int> area, ModuleTone tone = ModuleTone::Dark);

    juce::Slider& addKnob (const juce::String& paramId, const juce::String& label,
                           juce::Rectangle<int> area);
    juce::Slider& addKnob (const juce::String& paramId, const juce::String& label,
                           juce::Point<int> centre, int diameter);
    juce::Slider& addFader (const juce::String& paramId, const juce::String& label,
                            juce::Rectangle<int> area);
    juce::ComboBox& addChoice (const juce::String& paramId, const juce::String& label,
                               juce::Rectangle<int> area);
    HardwareSwitch& addSwitch (const juce::String& paramId, const juce::String& label,
                               juce::Rectangle<int> area, bool vertical = false);
    LedButton& addLed (const juce::String& paramId, const juce::String& label,
                       juce::Rectangle<int> area);
    Jack& addJack (const juce::String& patchId, juce::Point<int> centre, int diameter = 17);
    Jack& addJack (const juce::String& patchId, juce::Rectangle<int> area);
    LcdDisplay& addLcd (juce::Rectangle<int> area);
    juce::TextButton& addButton (const juce::String& text, juce::Rectangle<int> area,
                                 std::function<void()> onClick);
    SourceMatrix& addSourceMatrix (std::vector<juce::String> sourceParameterIds,
                                   int rows, juce::Rectangle<int> area);

    juce::Label& addCaption (const juce::String& text, juce::Rectangle<int> area,
                             juce::Justification justification = juce::Justification::centred,
                             float fontSize = 10.0f, juce::Colour colour = juce::Colour (0xffa8a49c),
                             bool bold = false);

    void clearControls();
    void refreshPatchCables();
    void pollPatchCables();

    juce::Slider& knobFor (const juce::String& paramId);

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;

    std::function<void()> onPatchChanged;

protected:
    void handleJackClicked (Jack& jack);
    void handleJackRightClicked (Jack& jack);
    Jack* findJack (int patchIndex) const;
    juce::Colour cableColour (int index) const;

    struct ModuleFrame
    {
        juce::String title;
        juce::String subtitle;
        juce::Rectangle<int> bounds;
        ModuleTone tone = ModuleTone::Dark;
    };

    std::vector<ModuleFrame> modules_;

    enum class Finish { Matriarch, PolyBrute, MoogOne };
    Finish finish_ = Finish::Matriarch;

private:
    juce::RangedAudioParameter* parameterFor (const juce::String& paramId) const;
    void addLabel (const juce::String& text, juce::Rectangle<int> area, float fontSize,
                   juce::Colour colour, bool bold);

    EditorHost& host_;
    std::unique_ptr<juce::AudioParameterFloat> fallbackParameter_;
    juce::String title_;
    juce::String brandMain_, brandSub_;
    int designWidth_ = 1800;
    int designHeight_ = 700;
    juce::Colour accent_;

    std::vector<std::unique_ptr<juce::Label>> labels_;
    std::vector<std::unique_ptr<juce::Slider>> sliders_;
    std::vector<std::unique_ptr<juce::ComboBox>> combos_;
    std::vector<std::unique_ptr<HardwareSwitch>> switches_;
    std::vector<std::unique_ptr<LedButton>> leds_;
    std::vector<std::unique_ptr<LcdDisplay>> lcds_;
    std::vector<std::unique_ptr<juce::TextButton>> buttons_;
    std::vector<std::unique_ptr<SourceMatrix>> matrices_;
    std::vector<std::unique_ptr<Jack>> jacks_;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments_;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments_;

    std::vector<PatchCable> cables_;
    int selectedJack_ = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelBase)
};

} // namespace triad::ui
