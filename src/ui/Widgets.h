#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace triad::ui
{

/** Two- to four-position hardware-style selector bound to a choice/float parameter. */
class HardwareSwitch : public juce::Component, public juce::SettableTooltipClient
{
public:
    enum class Orientation { Horizontal, Vertical };

    HardwareSwitch (juce::RangedAudioParameter& parameter, Orientation orientation);
    ~HardwareSwitch() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;

    void setAccent (juce::Colour colour);

private:
    void setPositionFromValue (float value);
    void handleClick (juce::Point<float> position);

    juce::RangedAudioParameter& parameter_;
    std::unique_ptr<juce::ParameterAttachment> attachment_;
    juce::Colour accent_ { 0xffcf4b2e };
    Orientation orientation_;
    int positions_ = 2;
    int current_ = 0;
    juce::StringArray labels_;
};

/** Illuminated panel button bound to a 0/1 parameter. */
class LedButton : public juce::Component, public juce::SettableTooltipClient
{
public:
    LedButton (juce::RangedAudioParameter& parameter, juce::String label);
    ~LedButton() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void setAccent (juce::Colour colour);

private:
    void setOn (float value);

    juce::RangedAudioParameter& parameter_;
    std::unique_ptr<juce::ParameterAttachment> attachment_;
    juce::String label_;
    juce::Colour accent_ { 0xffcf4b2e };
    bool on_ = false;
};

/** A patch point with a drawn socket. */
class Jack : public juce::Component, public juce::SettableTooltipClient
{
public:
    Jack (int patchIndex, bool isOutput, juce::String name, juce::Colour accent);

    int patchIndex() const noexcept { return patchIndex_; }
    bool isOutput() const noexcept { return isOutput_; }
    const juce::String& jackName() const noexcept { return name_; }

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    void setSelected (bool selected);
    void setCableCount (int count);

    std::function<void (Jack&)> onJackClicked;
    std::function<void (Jack&)> onJackRightClicked;

private:
    int patchIndex_ = -1;
    bool isOutput_ = false;
    bool selected_ = false;
    bool hover_ = false;
    int cableCount_ = 0;
    juce::String name_;
    juce::Colour accent_;
};

/** 12 x 8 performance matrix: each column selects the mod-matrix source for one slot. */
class SourceMatrix : public juce::Component, public juce::SettableTooltipClient, private juce::Timer
{
public:
    SourceMatrix (juce::AudioProcessorValueTreeState& state,
                  std::vector<juce::String> sourceParameterIds,
                  int rowCount, int columnCount);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

    std::function<void (int column)> onColumnSelected;
    int selectedColumn() const { return selectedColumn_; }

private:
    void timerCallback() override;
    int rowAt (juce::Point<int> position) const;
    int columnAt (juce::Point<int> position) const;

    juce::AudioProcessorValueTreeState& state_;
    std::vector<juce::String> ids_;
    std::vector<std::atomic<float>*> pointers_;
    int rows_ = 12;
    int columns_ = 8;
    int selectedColumn_ = 0;
};

/** LCD-style readout used for the PolyBrute matrix pages and the One console. */
class LcdDisplay : public juce::Component
{
public:
    void setText (juce::String header, juce::String body, juce::String footer = {});
    void paint (juce::Graphics&) override;

private:
    juce::String header_, body_, footer_;
};

} // namespace triad::ui
