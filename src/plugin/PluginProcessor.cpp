#include "plugin/PluginProcessor.h"

#include "ui/Editor.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace triad
{

#ifndef TRIAD_INSTRUMENT_KIND
 #define TRIAD_INSTRUMENT_KIND 0
#endif

namespace
{
juce::AudioProcessorValueTreeState::ParameterLayout buildLayout (const ModelDefinition& definition)
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    std::vector<std::unique_ptr<juce::AudioProcessorParameterGroup>> groups;
    std::map<juce::String, size_t> groupIndex;

    auto groupFor = [&] (const juce::String& name) -> juce::AudioProcessorParameterGroup&
    {
        auto found = groupIndex.find (name);
        if (found != groupIndex.end())
            return *groups[found->second];

        auto group = std::make_unique<juce::AudioProcessorParameterGroup> (
            name.toLowerCase().replaceCharacter (' ', '_'), name, " / ");
        groupIndex[name] = groups.size();
        groups.push_back (std::move (group));
        return *groups.back();
    };

    for (const auto& spec : definition.params)
    {
        auto& group = groupFor (juce::String (spec.group));
        const juce::ParameterID id { juce::String (spec.id), 1 };

        if (spec.curve == ParamCurve::Discrete)
        {
            juce::StringArray choices;
            for (const auto& choice : spec.choices)
                choices.add (choice);

            if (choices.isEmpty())
            {
                choices.add ("Off");
                choices.add ("On");
            }

            const auto defaultIndex = juce::jlimit (0, choices.size() - 1,
                                                    (int) std::lround (spec.def));

            group.addChild (std::make_unique<juce::AudioParameterChoice> (
                id, juce::String (spec.name), choices, defaultIndex));
            continue;
        }

        juce::NormalisableRange<float> range (spec.min, spec.max, 0.0f);
        if (spec.curve == ParamCurve::Log && spec.min > 0.0f && spec.max > spec.min)
            range.setSkewForCentre (std::sqrt (spec.min * spec.max));

        const auto unit = juce::String (spec.unit);

        auto formatValue = [unit] (float value, int maximumStringLength) -> juce::String
        {
            juce::String text;

            if (unit == "Hz")
            {
                if (value >= 1000.0f)
                    text = juce::String (value / 1000.0f, value >= 10000.0f ? 1 : 2) + " kHz";
                else
                    text = juce::String (value, value < 10.0f ? 2 : 0) + " Hz";
            }
            else if (unit == "s")
            {
                if (value < 1.0f)
                    text = juce::String (value * 1000.0f, 0) + " ms";
                else
                    text = juce::String (value, 2) + " s";
            }
            else if (unit == "st")
            {
                text = (value >= 0.0f ? "+" : "") + juce::String (value, 2) + " st";
            }
            else if (unit == "cents")
            {
                text = juce::String (value, 1) + " ct";
            }
            else if (unit.isNotEmpty())
            {
                text = juce::String (value, 2) + " " + unit;
            }
            else
            {
                text = juce::String (value, 2);
            }

            return text.substring (0, maximumStringLength);
        };

        auto attributes = juce::AudioParameterFloatAttributes()
                              .withLabel (unit)
                              .withStringFromValueFunction (formatValue);

        group.addChild (std::make_unique<juce::AudioParameterFloat> (
            id, juce::String (spec.name), range, juce::jlimit (spec.min, spec.max, spec.def),
            attributes));
    }

    for (auto& group : groups)
        layout.add (std::move (group));

    return layout;
}
} // namespace

TriadAudioProcessor::TriadAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      kind_ ((InstrumentKind) TRIAD_INSTRUMENT_KIND),
      engine_ (makeEngine (kind_)),
      apvts_ (*this, nullptr, "PARAMETERS", buildLayout (engine_->definition()))
{
    aliveFlag_ = std::make_shared<std::atomic<bool>> (true);

    const auto& definition = engine_->definition();
    const auto parameterCount = (int) definition.params.size();

    naturalValues_.assign ((size_t) parameterCount, 0.0f);
    morphScratch_.assign ((size_t) parameterCount, 0.0f);
    morphValues_ = std::make_unique<std::atomic<float>[]> ((size_t) parameterCount);

    for (int i = 0; i < parameterCount; ++i)
    {
        const auto& spec = definition.params[(size_t) i];
        parameterPointers_.push_back (apvts_.getRawParameterValue (spec.id));
        naturalValues_[(size_t) i] = spec.def;
        morphValues_[(size_t) i].store (spec.def);
    }

    factoryPresets_ = factoryPresetsFor (definition.id);

    arpEnabledIndex_ = findParameterBySuffix (".arp.enabled");
    arpModeIndex_ = findParameterBySuffix (".arp.mode");
    arpRateIndex_ = findParameterBySuffix (".arp.rate");
    arpOctavesIndex_ = findParameterBySuffix (".arp.octaves");
    arpHoldIndex_ = findParameterBySuffix (".arp.hold");
    arpLengthIndex_ = findParameterBySuffix (".arp.length");
    arpSwingIndex_ = findParameterBySuffix (".arp.swing");
    morphParameterIndex_ = findMorphParameter();

    sequencer_.prepare (44100.0);
    setLatencySamples (0);
}

TriadAudioProcessor::~TriadAudioProcessor()
{
    if (aliveFlag_ != nullptr)
        aliveFlag_->store (false);
}

int TriadAudioProcessor::findParameterBySuffix (const juce::String& suffix) const
{
    const auto& params = engine_->definition().params;

    for (int i = 0; i < (int) params.size(); ++i)
        if (juce::String (params[(size_t) i].id).endsWith (suffix))
            return i;

    return -1;
}

int TriadAudioProcessor::findMorphParameter() const
{
    const auto& params = engine_->definition().params;

    for (int i = 0; i < (int) params.size(); ++i)
    {
        const auto id = juce::String (params[(size_t) i].id);
        if (id.endsWith ("morph") && params[(size_t) i].curve != ParamCurve::Discrete)
            return i;
    }

    return -1;
}

bool TriadAudioProcessor::hasMorph() const
{
    return kind_ == InstrumentKind::PolyBrute;
}

bool TriadAudioProcessor::hasPatching() const
{
    return kind_ == InstrumentKind::Matriarch;
}

bool TriadAudioProcessor::hasParts() const
{
    return kind_ == InstrumentKind::MoogOne;
}

void TriadAudioProcessor::setSelectedPart (int part)
{
    selectedPart_.store (juce::jlimit (1, 3, part));

    if (auto* parameter = apvts_.getParameter ("one.master.selectedPart"))
    {
        const auto value = (float) juce::jlimit (1, 3, part);
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
    }
}

void TriadAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine_->prepare (sampleRate, samplesPerBlock);
    sequencer_.prepare (sampleRate);
    keyboardState_.reset();

    dummyRight_.assign ((size_t) juce::jmax (8192, samplesPerBlock), 0.0f);
    readParameters();
    engine_->setParameters (naturalValues_.data(), (int) naturalValues_.size());

    lastPitchBendSent_ = 0.0f;
    lastModWheelSent_ = -1.0f;
    lastSequencerEnabled_ = false;
    scheduledCount_ = 0;
}

const juce::String TriadAudioProcessor::getName() const
{
    return definition().displayName;
}

juce::AudioProcessorEditor* TriadAudioProcessor::createEditor()
{
    return new ui::TriadEditor (*this);
}

int TriadAudioProcessor::getNumPrograms()
{
    return factoryPresets_.empty() ? 1 : (int) factoryPresets_.size() + 1;
}

int TriadAudioProcessor::getCurrentProgram()
{
    return currentProgram_;
}

void TriadAudioProcessor::setCurrentProgram (int index)
{
    currentProgram_ = juce::jlimit (0, getNumPrograms() - 1, index);

    if (currentProgram_ == 0)
        initPatch();
    else
        loadFactoryPreset (currentProgram_ - 1);
}

const juce::String TriadAudioProcessor::getProgramName (int index)
{
    if (index <= 0)
        return "INIT";

    const auto presetIndex = index - 1;
    if (presetIndex < (int) factoryPresets_.size())
        return factoryPresets_[(size_t) presetIndex].name;

    return {};
}

void TriadAudioProcessor::changeProgramName (int, const juce::String&) {}

void TriadAudioProcessor::readParameters()
{
    for (size_t i = 0; i < parameterPointers_.size(); ++i)
    {
        if (auto* pointer = parameterPointers_[i])
            naturalValues_[i] = pointer->load (std::memory_order_relaxed);
    }
}

bool TriadAudioProcessor::toMidiEvent (const juce::MidiMessage& message, MidiEvent& out)
{
    out.channel = juce::jlimit (0, 15, message.getChannel() - 1);

    if (message.isNoteOn())
    {
        out.type = MidiEventType::NoteOn;
        out.note = message.getNoteNumber();
        out.value = message.getFloatVelocity();
        return true;
    }

    if (message.isNoteOff())
    {
        out.type = MidiEventType::NoteOff;
        out.note = message.getNoteNumber();
        out.value = 0.0f;
        return true;
    }

    if (message.isAllNotesOff())
    {
        out.type = MidiEventType::AllNotesOff;
        return true;
    }

    if (message.isAllSoundOff())
    {
        out.type = MidiEventType::AllSoundOff;
        return true;
    }

    if (message.isPitchWheel())
    {
        out.type = MidiEventType::PitchBend;
        out.value = ((float) message.getPitchWheelValue() - 8192.0f) / 8192.0f;
        return true;
    }

    if (message.isChannelPressure())
    {
        out.type = MidiEventType::ChannelPressure;
        out.value = (float) message.getChannelPressureValue() / 127.0f;
        return true;
    }

    if (message.isAftertouch())
    {
        out.type = MidiEventType::ChannelPressure;
        out.value = (float) message.getAfterTouchValue() / 127.0f;
        return true;
    }

    if (message.isController())
    {
        const auto controller = message.getControllerNumber();
        const auto value = (float) message.getControllerValue() / 127.0f;

        if (controller == 1)
        {
            out.type = MidiEventType::ModWheel;
            out.value = value;
            return true;
        }

        if (controller == 64)
        {
            out.type = MidiEventType::SustainPedal;
            out.value = value;
            return true;
        }

        out.type = MidiEventType::ControlChange;
        out.controller = controller;
        out.value = value;
        return true;
    }

    return false;
}

void TriadAudioProcessor::addScheduledEvent (int offset, const MidiEvent& event)
{
    if (scheduledCount_ >= (int) scheduled_.size())
        return;

    scheduled_[(size_t) scheduledCount_].offset = offset;
    scheduled_[(size_t) scheduledCount_].event = event;
    ++scheduledCount_;
}

void TriadAudioProcessor::updateSequencerFromParameters (const TransportInfo& transport)
{
    if (arpEnabledIndex_ < 0)
        return;

    const auto enabled = naturalValues_[(size_t) arpEnabledIndex_] > 0.5f;
    const auto hold = arpHoldIndex_ >= 0 && naturalValues_[(size_t) arpHoldIndex_] > 0.5f;

    sequencer_.setTempo (transport.bpm > 0.0 ? transport.bpm : 120.0);
    sequencer_.setMode (0);

    if (enabled)
    {
        const auto mode = arpModeIndex_ >= 0
                              ? juce::jlimit (0, 5, (int) std::lround (naturalValues_[(size_t) arpModeIndex_]))
                              : 0;
        sequencer_.setMode (mode + 1);
        sequencer_.setRateIndex (arpRateIndex_ >= 0
                                     ? (int) std::lround (naturalValues_[(size_t) arpRateIndex_])
                                     : 3);
        sequencer_.setOctaves (arpOctavesIndex_ >= 0
                                   ? (int) std::lround (naturalValues_[(size_t) arpOctavesIndex_]) + 1
                                   : 1);
        sequencer_.setHold (hold);
        sequencer_.setPlaying (true);

        if (arpLengthIndex_ >= 0)
        {
            const auto isDiscrete = engine_->definition().params[(size_t) arpLengthIndex_].curve
                                    == ParamCurve::Discrete;
            sequencer_.setStepCount ((int) std::lround (naturalValues_[(size_t) arpLengthIndex_])
                                     + (isDiscrete ? 1 : 0));
        }

        if (arpSwingIndex_ >= 0)
            sequencer_.setSwing (naturalValues_[(size_t) arpSwingIndex_]);
    }

    if (lastArpHold_ && ! hold)
        sequencer_.setHoldActive (false);

    lastArpHold_ = hold;
}

void TriadAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numSamples = buffer.getNumSamples();
    const auto numChannels = buffer.getNumChannels();

    for (int channel = 2; channel < numChannels; ++channel)
        buffer.clear (channel, 0, numSamples);

    if (numSamples <= 0)
        return;

    if (numChannels == 0)
    {
        midi.clear();
        return;
    }

    readParameters();
    engine_->setParameters (naturalValues_.data(), (int) naturalValues_.size());

    if (hasMorph() && morphValues_ != nullptr)
    {
        for (size_t i = 0; i < morphScratch_.size(); ++i)
            morphScratch_[i] = morphValues_[i].load (std::memory_order_relaxed);

        engine_->setMorphSnapshot (morphScratch_.data(), (int) morphScratch_.size());

        if (morphParameterIndex_ >= 0)
            engine_->setMorphAmount (naturalValues_[(size_t) morphParameterIndex_]);
    }

    TransportInfo transport;
    if (auto* playHead = getPlayHead())
    {
        if (const auto position = playHead->getPosition())
        {
            transport.bpm = position->getBpm().orFallback (120.0);
            transport.ppqPosition = position->getPpqPosition().orFallback (0.0);
            transport.isPlaying = position->getIsPlaying();
        }
    }

    engine_->setTransport (transport);
    updateSequencerFromParameters (transport);

    keyboardState_.processNextMidiBuffer (midi, 0, numSamples, true);

    scheduledCount_ = 0;

    // On-screen wheels are applied at the start of the block.
    const auto uiBend = uiPitchBend_.load();
    if (std::abs (uiBend - lastPitchBendSent_) > 1.0e-4f)
    {
        MidiEvent event;
        event.type = MidiEventType::PitchBend;
        event.value = uiBend;
        addScheduledEvent (0, event);
        lastPitchBendSent_ = uiBend;
    }

    const auto uiMod = uiModWheel_.load();
    if (std::abs (uiMod - lastModWheelSent_) > 1.0e-4f)
    {
        MidiEvent event;
        event.type = MidiEventType::ModWheel;
        event.value = uiMod;
        addScheduledEvent (0, event);
        lastModWheelSent_ = uiMod;
    }

    const auto sequencerEnabled = arpEnabledIndex_ >= 0
                               && naturalValues_[(size_t) arpEnabledIndex_] > 0.5f;

    if (sequencerEnabled != lastSequencerEnabled_)
    {
        MidiEvent allOff;
        allOff.type = MidiEventType::AllNotesOff;
        addScheduledEvent (0, allOff);
        sequencer_.allNotesOff();
        lastSequencerEnabled_ = sequencerEnabled;
    }

    if (sequencerEnabled)
    {
        for (const auto metadata : midi)
        {
            const auto message = metadata.getMessage();
            MidiEvent event;

            if (message.isNoteOn())
            {
                sequencer_.noteOn (message.getNoteNumber(), message.getFloatVelocity());
                continue;
            }

            if (message.isNoteOff())
            {
                sequencer_.noteOff (message.getNoteNumber());
                continue;
            }

            if (message.isAllNotesOff() || message.isAllSoundOff())
            {
                sequencer_.allNotesOff();
                MidiEvent off;
                off.type = MidiEventType::AllNotesOff;
                addScheduledEvent (metadata.samplePosition, off);
                continue;
            }

            if (toMidiEvent (message, event))
                addScheduledEvent (metadata.samplePosition, event);
        }

        PatternSequencer::Event events[PatternSequencer::maxEvents];
        const auto count = sequencer_.process (numSamples, events, PatternSequencer::maxEvents);

        for (int i = 0; i < count; ++i)
        {
            MidiEvent event;
            event.type = events[i].noteOn ? MidiEventType::NoteOn : MidiEventType::NoteOff;
            event.note = events[i].note;
            event.value = events[i].velocity;
            addScheduledEvent (events[i].sampleOffset, event);
        }
    }
    else
    {
        for (const auto metadata : midi)
        {
            MidiEvent event;
            if (toMidiEvent (metadata.getMessage(), event))
                addScheduledEvent (metadata.samplePosition, event);
        }
    }

    std::sort (scheduled_.begin(), scheduled_.begin() + scheduledCount_,
               [] (const ScheduledEvent& a, const ScheduledEvent& b)
               {
                   return a.offset < b.offset;
               });

    if ((size_t) numSamples > dummyRight_.size())
        dummyRight_.resize ((size_t) numSamples);

    auto* left = buffer.getWritePointer (0);
    float* right = numChannels > 1 ? buffer.getWritePointer (1) : dummyRight_.data();

    int position = 0;

    for (int i = 0; i < scheduledCount_; ++i)
    {
        const auto offset = juce::jlimit (position, numSamples, scheduled_[(size_t) i].offset);

        if (offset > position)
        {
            engine_->render (left + position, right + position, offset - position);
            position = offset;
        }

        engine_->processMidi (scheduled_[(size_t) i].event);
    }

    if (position < numSamples)
        engine_->render (left + position, right + position, numSamples - position);

}

void TriadAudioProcessor::captureMorphB()
{
    for (size_t i = 0; i < parameterPointers_.size(); ++i)
    {
        const auto value = parameterPointers_[i] != nullptr
                               ? parameterPointers_[i]->load (std::memory_order_relaxed)
                               : engine_->definition().params[i].def;
        morphValues_[i].store (value);
    }
}

void TriadAudioProcessor::swapMorphB()
{
    const auto& params = engine_->definition().params;

    std::vector<float> oldA (naturalValues_.size());
    for (size_t i = 0; i < naturalValues_.size(); ++i)
    {
        oldA[i] = parameterPointers_[i] != nullptr
                      ? parameterPointers_[i]->load (std::memory_order_relaxed)
                      : params[i].def;
    }

    for (size_t i = 0; i < params.size(); ++i)
    {
        const auto morphValue = morphValues_[i].load();
        setParameterNatural (juce::String (params[i].id), morphValue);
        morphValues_[i].store (oldA[i]);
    }
}

void TriadAudioProcessor::setParameterNatural (const juce::String& id, float value)
{
    if (auto* parameter = apvts_.getParameter (id))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
}

std::vector<PatchCable> TriadAudioProcessor::patchCables() const
{
    const juce::ScopedLock lock (patchLock_);
    return patchCables_;
}

void TriadAudioProcessor::setPatchCable (int from, int to, bool connected)
{
    const auto patchCount = (int) engine_->definition().patchPoints.size();
    if (from < 0 || from >= patchCount || to < 0 || to >= patchCount || from == to)
        return;

    if (! hasPatching())
        return;

    const juce::ScopedLock lock (patchLock_);

    if (connected)
    {
        for (auto it = patchCables_.begin(); it != patchCables_.end();)
        {
            if (it->to == to)
            {
                engine_->setPatchCable (it->from, it->to, false);
                it = patchCables_.erase (it);
            }
            else
            {
                ++it;
            }
        }

        const auto duplicate = std::find_if (patchCables_.begin(), patchCables_.end(),
                                             [from, to] (const PatchCable& cable)
                                             {
                                                 return cable.from == from && cable.to == to;
                                             });

        if (duplicate == patchCables_.end())
            patchCables_.push_back ({ from, to });

        engine_->setPatchCable (from, to, true);
    }
    else
    {
        engine_->setPatchCable (from, to, false);

        patchCables_.erase (std::remove_if (patchCables_.begin(), patchCables_.end(),
                                            [from, to] (const PatchCable& cable)
                                            {
                                                return cable.from == from && cable.to == to;
                                            }),
                            patchCables_.end());
    }
}

void TriadAudioProcessor::clearPatchCables()
{
    const juce::ScopedLock lock (patchLock_);
    patchCables_.clear();
    engine_->clearPatchCables();
}

int TriadAudioProcessor::factoryPresetCount() const
{
    return (int) factoryPresets_.size();
}

juce::String TriadAudioProcessor::factoryPresetName (int index) const
{
    if (index >= 0 && index < (int) factoryPresets_.size())
        return factoryPresets_[(size_t) index].name;

    return {};
}

void TriadAudioProcessor::initPatch()
{
    for (const auto& spec : engine_->definition().params)
        setParameterNatural (juce::String (spec.id), spec.def);

    if (hasMorph())
    {
        for (size_t i = 0; i < engine_->definition().params.size(); ++i)
            morphValues_[i].store (engine_->definition().params[i].def);
    }

    if (hasPatching())
        clearPatchCables();

    currentPresetName_ = "INIT";
    currentProgram_ = 0;
}

void TriadAudioProcessor::applyPresetValues (const std::vector<std::pair<juce::String, float>>& values,
                                             const juce::String& presetName, bool resetFirst)
{
    if (resetFirst)
        initPatch();

    for (const auto& entry : values)
        setParameterNatural (entry.first, entry.second);

    currentPresetName_ = presetName;
}

void TriadAudioProcessor::loadFactoryPreset (int index)
{
    if (index < 0 || index >= (int) factoryPresets_.size())
        return;

    const auto& preset = factoryPresets_[(size_t) index];
    applyPresetValues (preset.values, preset.name, true);
    currentProgram_ = index + 1;
}

juce::String TriadAudioProcessor::currentPresetName() const
{
    return currentPresetName_;
}

void TriadAudioProcessor::encodeState (juce::ValueTree& state)
{
    state.setProperty ("instrument", juce::String (engine_->definition().id), nullptr);
    state.setProperty ("formatVersion", 2, nullptr);
    state.setProperty ("presetName", currentPresetName_, nullptr);

    juce::String cables;
    {
        const juce::ScopedLock lock (patchLock_);
        for (const auto& cable : patchCables_)
            cables << cable.from << ":" << cable.to << ";";
    }
    state.setProperty ("patchCables", cables, nullptr);

    if (morphValues_ != nullptr)
    {
        juce::ValueTree morph ("MORPH_B");
        for (size_t i = 0; i < engine_->definition().params.size(); ++i)
            morph.setProperty ("v" + juce::String ((int) i), morphValues_[i].load(), nullptr);
        state.addChild (morph, -1, nullptr);
    }

    juce::ValueTree sequence ("SEQUENCER");
    for (int i = 0; i < PatternSequencer::maxSteps; ++i)
    {
        sequence.setProperty ("a" + juce::String (i), sequencer_.isStepActive (i), nullptr);
        sequence.setProperty ("n" + juce::String (i), sequencer_.stepNote (i), nullptr);
        sequence.setProperty ("v" + juce::String (i), sequencer_.stepVelocity (i), nullptr);
    }
    state.addChild (sequence, -1, nullptr);
}

void TriadAudioProcessor::decodeState (const juce::ValueTree& state)
{
    const auto morphTree = state.getChildWithName ("MORPH_B");
    if (morphTree.isValid() && morphValues_ != nullptr)
    {
        for (size_t i = 0; i < engine_->definition().params.size(); ++i)
        {
            const auto key = "v" + juce::String ((int) i);
            if (morphTree.hasProperty (key))
                morphValues_[i].store ((float) morphTree.getProperty (key));
        }
    }

    const auto sequence = state.getChildWithName ("SEQUENCER");
    if (sequence.isValid())
    {
        for (int i = 0; i < PatternSequencer::maxSteps; ++i)
        {
            const auto index = juce::String (i);
            if (sequence.hasProperty ("a" + index))
                sequencer_.setStepActive (i, (bool) sequence.getProperty ("a" + index));
            if (sequence.hasProperty ("n" + index))
                sequencer_.setStepNote (i, (int) sequence.getProperty ("n" + index));
            if (sequence.hasProperty ("v" + index))
                sequencer_.setStepVelocity (i, (int) sequence.getProperty ("v" + index));
        }
    }

    currentPresetName_ = state.getProperty ("presetName", "INIT").toString();

    if (hasPatching())
    {
        clearPatchCables();

        const auto encoded = state.getProperty ("patchCables").toString();
        const auto entries = juce::StringArray::fromTokens (encoded, ";", "");

        for (const auto& entry : entries)
        {
            if (entry.isEmpty())
                continue;

            const auto numbers = juce::StringArray::fromTokens (entry, ":", "");
            if (numbers.size() == 2)
                setPatchCable (numbers[0].getIntValue(), numbers[1].getIntValue(), true);
        }
    }
}

void TriadAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts_.copyState();
    encodeState (state);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void TriadAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr)
        return;

    auto state = juce::ValueTree::fromXml (*xml);
    if (! state.isValid())
        return;

    const auto instrument = state.getProperty ("instrument").toString();
    if (instrument.isNotEmpty() && instrument != juce::String (engine_->definition().id))
        return;

    apvts_.replaceState (state);
    decodeState (state);
    readParameters();

    if (auto* selected = apvts_.getParameter ("one.master.selectedPart"))
        selectedPart_.store (juce::jlimit (1, 3, (int) std::lround (
            selected->convertFrom0to1 (selected->getValue()))));

    if (hasMorph() && morphParameterIndex_ >= 0)
        engine_->setMorphAmount (naturalValues_[(size_t) morphParameterIndex_]);

    if (auto* editor = getActiveEditor())
        editor->repaint();
}

void TriadAudioProcessor::savePreset()
{
    auto start = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                     .getChildFile ("Analog Triad")
                     .getChildFile (definition().displayName + ".triadpreset");

    fileChooser_ = std::make_unique<juce::FileChooser> ("Save preset", start, "*.triadpreset");

    auto alive = aliveFlag_;
    fileChooser_->launchAsync (juce::FileBrowserComponent::saveMode
                                   | juce::FileBrowserComponent::canSelectFiles
                                   | juce::FileBrowserComponent::warnAboutOverwriting,
                               [this, alive] (const juce::FileChooser& chooser)
                               {
                                   if (! alive->load())
                                       return;

                                   auto file = chooser.getResult();
                                   if (file == juce::File())
                                       return;

                                   savePresetFile (file.withFileExtension ("triadpreset"));
                               });
}

void TriadAudioProcessor::loadPreset()
{
    auto start = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                     .getChildFile ("Analog Triad");
    fileChooser_ = std::make_unique<juce::FileChooser> ("Load preset", start, "*.triadpreset");

    auto alive = aliveFlag_;
    fileChooser_->launchAsync (juce::FileBrowserComponent::openMode
                                   | juce::FileBrowserComponent::canSelectFiles,
                               [this, alive] (const juce::FileChooser& chooser)
                               {
                                   if (! alive->load())
                                       return;

                                   const auto file = chooser.getResult();
                                   if (file.existsAsFile())
                                       loadPresetFile (file);
                               });
}

void TriadAudioProcessor::savePresetFile (const juce::File& file)
{
    juce::MemoryBlock block;
    getStateInformation (block);
    file.replaceWithData (block.getData(), block.getSize());
    currentPresetName_ = file.getFileNameWithoutExtension();
}

void TriadAudioProcessor::loadPresetFile (const juce::File& file)
{
    juce::MemoryBlock block;
    if (! file.loadFileAsData (block))
        return;

    setStateInformation (block.getData(), (int) block.getSize());
    currentPresetName_ = file.getFileNameWithoutExtension();
}

} // namespace triad
