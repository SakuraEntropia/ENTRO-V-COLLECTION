#pragma once

#include "ui/EditorHost.h"
#include "ui/KeyboardPanel.h"
#include "ui/LookAndFeel.h"
#include "ui/Panels.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace triad::ui
{

class TriadEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit TriadEditor (EditorHost& host);
    ~TriadEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class HeaderBar;

    void timerCallback() override;

    EditorHost& host_;
    TriadLookAndFeel lookAndFeel_;
    std::unique_ptr<HeaderBar> header_;
    std::unique_ptr<PanelBase> panel_;
    KeyboardPanel keyboard_;
    juce::TooltipWindow tooltip_ { this, 700 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriadEditor)
};

} // namespace triad::ui
