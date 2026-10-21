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

class MorphPad;

class PolyBrutePanel final : public PanelBase
{
public:
    explicit PolyBrutePanel (EditorHost& host);
    ~PolyBrutePanel() override;
    void paint (juce::Graphics&) override;

private:
    std::unique_ptr<MorphPad> morphPad_;
};

class OnePanel final : public PanelBase
{
public:
    explicit OnePanel (EditorHost& host);
    void rebuildForPart();
};

std::unique_ptr<PanelBase> createInstrumentPanel (EditorHost& host);

} // namespace triad::ui
