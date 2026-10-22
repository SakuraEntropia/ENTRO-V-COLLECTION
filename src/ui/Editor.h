#pragma once

#include "ui/EditorHost.h"
#include "ui/KeyboardPanel.h"
#include "ui/LookAndFeel.h"
#include "ui/Panels.h"
#include "ui/PresetBrowser.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>

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
    bool keyPressed (const juce::KeyPress&) override;
    void updatePhysicalKeyboard();

public:
    void setZoom (float zoom);

private:
    EditorHost& host_;
    TriadLookAndFeel lookAndFeel_;
    std::unique_ptr<HeaderBar> header_;
    class Backdrop final : public juce::Component
    {
    public:
        void paint (juce::Graphics&) override;
    };

    void openPresetBrowser();
    void closePresetBrowser();

    std::unique_ptr<PanelBase> panel_;
    std::unique_ptr<PresetBrowser> presetBrowser_;
    Backdrop browserBackdrop_;
    juce::Viewport viewport_;
    juce::Component canvas_;
    KeyboardPanel keyboard_;
    std::unique_ptr<ContinuumSurface> continuumSurface_;
    std::unique_ptr<juce::Component> continuumMonitor_;
    std::array<bool, 13> physicalKeys_ {};
    bool layoutDumped_ = false;
    float zoom_ = 0.0f;           // 0 = fit
    float currentFitScale_ = 1.0f;
    juce::TooltipWindow tooltip_ { this, 700 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriadEditor)
};

} // namespace triad::ui
