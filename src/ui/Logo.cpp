#include "ui/Logo.h"

#include <cmath>

namespace triad::ui
{

namespace
{
constexpr float kGrid = 128.0f;
constexpr float kPi = 3.14159265358979323846f;

juce::Path railPath()
{
    juce::Path path;
    path.startNewSubPath (16.0f, 96.0f);
    path.lineTo (50.0f, 96.0f);
    path.lineTo (64.0f, 106.0f);
    path.lineTo (78.0f, 96.0f);
    path.lineTo (112.0f, 96.0f);
    return path;
}

void drawRail (juce::Graphics& g, juce::Colour colour)
{
    g.setColour (colour);
    g.strokePath (railPath(), juce::PathStrokeType (6.0f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
}
} // namespace

LogoKind logoKindForInstrument (const juce::String& instrumentId)
{
    if (instrumentId == "polybrute")
        return LogoKind::PolyBrute;

    if (instrumentId == "moogone" || instrumentId == "one")
        return LogoKind::One;

    if (instrumentId == "continuum")
        return LogoKind::Continuum;

    return LogoKind::Matriarch;
}

void drawInstrumentMark (juce::Graphics& g, juce::Rectangle<float> area, LogoKind kind,
                         juce::Colour accent, juce::Colour foreground)
{
    const auto size = juce::jmin (area.getWidth(), area.getHeight());
    if (size <= 0.5f)
        return;

    const auto square = area.withSizeKeepingCentre (size, size);
    const auto scale = size / kGrid;

    juce::Graphics::ScopedSaveState save (g);
    g.addTransform (juce::AffineTransform::scale (scale).translated (square.getX(), square.getY()));

    if (kind == LogoKind::Matriarch)
    {
        g.setColour (foreground);
        const float bars[][2] = { { 30.0f, 26.0f }, { 52.0f, 48.0f },
                                  { 76.0f, 48.0f }, { 98.0f, 26.0f } };

        for (const auto& bar : bars)
        {
            juce::Path line;
            line.startNewSubPath (bar[0], 94.0f);
            line.lineTo (bar[0], bar[1]);
            g.strokePath (line, juce::PathStrokeType (10.0f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }

        drawRail (g, accent);
        return;
    }

    if (kind == LogoKind::PolyBrute)
    {
        g.setColour (foreground);
        const float bars[][2] = { { 22.0f, 62.0f }, { 38.0f, 42.0f }, { 70.0f, 34.0f },
                                  { 86.0f, 50.0f }, { 102.0f, 66.0f } };

        for (const auto& bar : bars)
        {
            juce::Path line;
            line.startNewSubPath (bar[0], 94.0f);
            line.lineTo (bar[0], bar[1]);
            g.strokePath (line, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));
        }

        // The centre voice is the accent, like the Morph axis.
        g.setColour (accent);
        juce::Path centre;
        centre.startNewSubPath (54.0f, 94.0f);
        centre.lineTo (54.0f, 30.0f);
        g.strokePath (centre, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));

        drawRail (g, accent);
        return;
    }

    if (kind == LogoKind::Continuum)
    {
        juce::Path ribbon;
        ribbon.startNewSubPath (18.0f, 92.0f);
        ribbon.cubicTo (34.0f, 20.0f, 54.0f, 20.0f, 64.0f, 62.0f);
        ribbon.cubicTo (74.0f, 104.0f, 94.0f, 104.0f, 110.0f, 32.0f);
        g.setColour (foreground);
        g.strokePath (ribbon, juce::PathStrokeType (9.0f, juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));

        g.setColour (accent);
        g.fillEllipse (58.0f, 56.0f, 12.0f, 12.0f);
        return;
    }

    // ONE: three arcs sharing one centre pool.
    g.setColour (foreground);
    const std::pair<float, float> arcs[] = { { 108.0f, 192.0f }, { 228.0f, 312.0f },
                                             { 348.0f, 432.0f } };

    for (const auto& arc : arcs)
    {
        juce::Path path;
        path.addCentredArc (64.0f, 60.0f, 31.0f, 31.0f, 0.0f,
                            arc.first * kPi / 180.0f, arc.second * kPi / 180.0f, true);
        g.strokePath (path, juce::PathStrokeType (9.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    g.setColour (accent);
    g.fillEllipse (57.0f, 53.0f, 14.0f, 14.0f);
}

} // namespace triad::ui
