#pragma once

#include <juce_core/juce_core.h>

#include <string>
#include <utility>
#include <vector>

namespace triad
{

struct FactoryPreset
{
    juce::String name;
    std::vector<std::pair<juce::String, float>> values;
};

std::vector<FactoryPreset> factoryPresetsFor (const std::string& instrumentId);

} // namespace triad
