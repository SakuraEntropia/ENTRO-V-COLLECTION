#pragma once

#include <juce_core/juce_core.h>

#include <string>
#include <utility>
#include <vector>

namespace triad
{

struct PresetMatrixCell
{
    int source = 0;
    int destination = 0;
    int formula = 0;
    float amount = 0.0f;
};

struct FactoryPreset
{
    juce::String name;
    juce::String category;
    std::vector<std::pair<juce::String, float>> values;
    std::vector<PresetMatrixCell> matrix;
};

std::vector<FactoryPreset> factoryPresetsFor (const std::string& instrumentId);

} // namespace triad
