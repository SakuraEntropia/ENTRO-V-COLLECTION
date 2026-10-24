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
    std::vector<std::pair<juce::String, float>> morphB;
    std::vector<std::pair<juce::String, juce::String>> cables;
};

std::vector<FactoryPreset> studioPresets (const std::string& instrumentId);
std::vector<FactoryPreset> oneReferencePresets();
std::vector<FactoryPreset> analogReferencePresets(const std::string&);
std::vector<FactoryPreset> factoryPresetsFor (const std::string& instrumentId);

} // namespace triad
