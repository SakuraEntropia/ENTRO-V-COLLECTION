#include "plugin/PluginProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new triad::TriadAudioProcessor();
}
