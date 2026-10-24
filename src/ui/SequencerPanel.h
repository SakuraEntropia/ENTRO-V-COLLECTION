#pragma once
#include "ui/EditorHost.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
namespace triad::ui {
/** Optional software editor: the instrument's physical panel remains unchanged. */
class SequencerPanel final : public juce::Component, private juce::Timer {
public:
    SequencerPanel(EditorHost&,std::function<void()> close);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void refresh();
    void timerCallback() override;
    void startPattern();
    EditorHost& host_;
    juce::TextButton close_{"CLOSE"}, play_{"PLAY"}, clear_{"CLEAR BANK"};
    juce::ComboBox bank_, page_;
    juce::Slider length_, gate_;
    std::array<juce::ToggleButton,16> enabled_,tie_;
    std::array<juce::Label,16> velocity_;
    std::array<std::array<juce::Label,6>,16> notes_;
    int pageIndex_=0,lastStep_=-1;
    bool refreshing_=false;
};
}
