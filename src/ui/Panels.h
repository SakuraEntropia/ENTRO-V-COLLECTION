#pragma once

#include "ui/EditorHost.h"
#include "ui/PanelBase.h"
#include "ui/ContinuumPanel.h"

#include <memory>

namespace triad::ui
{

class MatriarchPanel final : public PanelBase
{
public:
    explicit MatriarchPanel (EditorHost& host);
};

class PolyBrutePanel final : public PanelBase
{
public:
    explicit PolyBrutePanel (EditorHost& host);
    ~PolyBrutePanel() override;
    void paint (juce::Graphics&) override;

private:
    std::unique_ptr<juce::Component> wheels_;
};

class OnePanel final : public PanelBase
{
public:
    explicit OnePanel (EditorHost& host);
    void rebuildForPart();
private:
    std::array<std::unique_ptr<OneWaveDisplay>, 3> waveformDisplays_;
};

std::unique_ptr<PanelBase> createInstrumentPanel (EditorHost& host);

} // namespace triad::ui
