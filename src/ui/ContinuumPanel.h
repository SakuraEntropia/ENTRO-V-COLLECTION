#pragma once

#include "ui/EditorHost.h"
#include "ui/PanelBase.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace triad::ui
{

/** 3D continuous playing surface: X = pitch, Y = timbre, wheel = pressure. */
class ContinuumSurface final : public juce::Component,
                               private juce::Timer
{
public:
    explicit ContinuumSurface (EditorHost& host);
    ~ContinuumSurface() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    void visibilityChanged() override;
    void focusLost (juce::Component::FocusChangeType) override;

private:
    void timerCallback() override;
    void updateFromPosition (juce::Point<float> position, bool allowNoteChange);
    void beginNote (juce::Point<float> position);
    void endNote();
    juce::Point<float> pointFor (int note, float expr, float timbre) const;
    float pitchForX (float x) const;
    juce::Rectangle<float> playingArea() const;

    EditorHost& host_;
    bool active_ = false;
    int note_ = 60;
    float exprPitch_ = 0.0f;
    float timbre_ = 0.5f;
    float pressure_ = 0.8f;
    juce::Point<float> lastPoint_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContinuumSurface)
};

/** 16 x 16 EIGEN MATRIX: click cycles the formula, drag changes the amount. */
class ContinuumMatrix final : public juce::Component, private juce::Timer
{
public:
    explicit ContinuumMatrix (EditorHost& host);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    juce::Rectangle<float> cellBounds (int source, int destination) const;
    juce::Point<int> cellAt (juce::Point<float> position) const;

    EditorHost& host_;
    int dragSource_ = -1;
    int dragDestination_ = -1;
    float dragStartAmount_ = 0.0f;
    float dragStartY_ = 0.0f;
    int dragFormula_ = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContinuumMatrix)
};

class ContinuumPanel final : public PanelBase
{
public:
    explicit ContinuumPanel (EditorHost& host);

private:
    std::unique_ptr<ContinuumSurface> surface_;
    std::unique_ptr<ContinuumMatrix> matrix_;
};

} // namespace triad::ui
