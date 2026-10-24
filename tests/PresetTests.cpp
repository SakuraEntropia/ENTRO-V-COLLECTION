#include "engines/Engines.h"
#include "plugin/Presets.h"

#include <cmath>
#include <cstdio>
#include <set>
#include <vector>

namespace
{
int failures = 0;

void check (bool condition, const char* message, const juce::String& detail = {})
{
    if (! condition)
    {
        std::printf ("FAIL: %s %s\n", message, detail.toRawUTF8());
        ++failures;
    }
}
} // namespace

int main()
{
    for (const auto kind : { triad::InstrumentKind::Matriarch,
                             triad::InstrumentKind::PolyBrute,
                             triad::InstrumentKind::MoogOne,
                             triad::InstrumentKind::Continuum })
    {
        const auto engine = triad::makeEngine (kind);
        const auto& definition = engine->definition();
        const auto presets = triad::factoryPresetsFor (definition.id);

        check (! presets.empty(), "no factory presets", definition.id);

        std::set<juce::String> names;
        int lastCategoryIndex = -1;
        int bassCount = 0;
        int studioCount = 0;
        int stageCount = 0;
        std::set<std::vector<int>> woodwindSignatures;
        std::set<std::vector<std::pair<juce::String, float>>> studioSignatures;
        bool hasReese = false;

        const juce::StringArray categoryOrder { "Bass", "Lead", "Brass", "Pad", "Strings",
                                                "Winds", "Vocal", "Keyboard", "Classic", "Other",
                                                "Percussion", "Tuned Percussion", "Processor",
                                                "Drone", "Midi", "CVC", "User", "Surface", "Texture", "Pluck",
                                                "Keys", "Organ", "Arp", "Sequence", "Split",
                                                "Utility", "FX" };

        for (const auto& preset : presets)
        {
            if (preset.name.endsWith("[Stage]")) {
                ++stageCount;
                check(studioSignatures.insert(preset.values).second, "duplicate Stage parameter payload", preset.name);
            }
            if (preset.name.endsWith("[Studio]")) {
                ++studioCount;
                check(studioSignatures.insert(preset.values).second, "duplicate Studio parameter payload", preset.name);
            }
            check (names.insert (preset.name).second, "duplicate preset name", preset.name);
            check (! preset.values.empty(), "empty preset", preset.name);
            check (preset.category.isNotEmpty(), "empty preset category", preset.name);

            const auto categoryIndex = categoryOrder.indexOf (preset.category);
            check (categoryIndex >= 0, "unknown preset category", preset.category);
            check (categoryIndex >= lastCategoryIndex, "preset categories not grouped", preset.name);
            lastCategoryIndex = juce::jmax (lastCategoryIndex, categoryIndex);

            if (preset.category == "Bass")
                ++bassCount;

            if (preset.name.containsIgnoreCase ("Reese"))
                hasReese = true;

            for (const auto& cell : preset.matrix)
            {
                check (cell.source >= 0 && cell.source < engine->matrixRows(),
                       "preset matrix source out of range", preset.name);
                check (cell.destination >= 0 && cell.destination < engine->matrixColumns(),
                       "preset matrix destination out of range", preset.name);
                check (cell.formula >= 0 && cell.formula < engine->matrixFormulaCount(),
                       "preset matrix formula out of range", preset.name);
                check (cell.amount >= 0.0f && cell.amount <= 1.0f,
                       "preset matrix amount out of range", preset.name);
            }

            for(const auto& entry:preset.morphB) {
                const auto* spec=definition.find(entry.first.toStdString());
                check(spec!=nullptr,"unknown B-side parameter",preset.name+" / "+entry.first);
                if(spec)check(std::isfinite(entry.second) && entry.second>=spec->min && entry.second<=spec->max,"B-side value outside range",preset.name+" / "+entry.first);
            }
            for(const auto& cable:preset.cables) {
                int from=definition.patchIndexOf(cable.first.toStdString()),to=definition.patchIndexOf(cable.second.toStdString());
                check(from>=0 && to>=0,"unknown preset cable",preset.name);
                if(from>=0 && to>=0)check(definition.patchPoints[from].isOutput && !definition.patchPoints[to].isOutput,"invalid preset cable direction",preset.name);
            }
            for (const auto& entry : preset.values)
            {
                const auto* spec = definition.find (entry.first.toStdString());

                if (spec == nullptr)
                {
                    check (false, "unknown parameter in preset", preset.name + " / " + entry.first);
                    continue;
                }

                if (! std::isfinite (entry.second))
                {
                    check (false, "non-finite preset value", preset.name + " / " + entry.first);
                    continue;
                }

                if (entry.second < spec->min || entry.second > spec->max)
                {
                    check (false, "preset value outside range",
                           preset.name + " / " + entry.first + " = " + juce::String (entry.second));
                    continue;
                }

                if (spec->curve == triad::ParamCurve::Discrete)
                {
                    const auto index = (int) std::lround (entry.second);
                    if (index < 0 || index >= (int) spec->choices.size())
                        check (false, "preset choice index out of range",
                               preset.name + " / " + entry.first);
                }
            }
        }

        check(stageCount == 32, "expected 32 Stage patches", definition.id);
        check(studioCount == 64, "expected 64 Studio patches", definition.id);
        check (presets.size() >= 12, "too few factory presets", definition.id);
        check (bassCount >= 2, "bass category too small", definition.id);
        check (hasReese, "missing Reese bass preset", definition.id);

        // Render each preset to make sure it is audible, finite and bounded.
        auto renderEngine = triad::makeEngine (kind);
        renderEngine->prepare (48000.0, 256);


        for (const auto& preset : presets)
        {
            std::vector<float> values (definition.params.size());

            for (size_t i = 0; i < values.size(); ++i)
                values[i] = definition.params[i].def;

            for (const auto& entry : preset.values)
            {
                const auto index = definition.indexOf (entry.first.toStdString());
                if (index >= 0)
                    values[(size_t) index] = entry.second;
            }

            renderEngine->reset();
            renderEngine->resetMatrix();
            renderEngine->clearPatchCables();
            renderEngine->setParameters (values.data(), (int) values.size());

            for (const auto& cell : preset.matrix)
                renderEngine->setMatrixCell (cell.source, cell.destination,
                                             cell.formula, cell.amount);

            for(const auto& cable:preset.cables) renderEngine->setPatchCable(definition.patchIndexOf(cable.first.toStdString()),definition.patchIndexOf(cable.second.toStdString()),true);
            if(!preset.morphB.empty()) {
                auto b=values;
                for(const auto& v:preset.morphB){int index=definition.indexOf(v.first.toStdString());if(index>=0)b[index]=v.second;}
                renderEngine->setMorphSnapshot(b.data(),(int)b.size());renderEngine->setMorphAmount(.5f);
            }
            triad::MidiEvent note;
            note.type = triad::MidiEventType::NoteOn;
            note.note = 60;
            note.value = 0.8f;
            renderEngine->processMidi (note);

            float left[256];
            float right[256];
            double energy = 0.0;
            float peak = 0.0f;
            bool finite = true;
            const bool checkWoodwind=definition.id=="continuum" && (preset.name=="WI - Clarinet" || preset.name=="WI - Chinese Clarinet" || preset.name=="WI - Spinning Oboe" || preset.name=="WI - Sine2Sax");
            std::vector<int> signature;

            for (int block = 0; block < (int) (0.75 * 48000.0 / 256.0); ++block)
            {
                renderEngine->render (left, right, 256);

                for (int i = 0; i < 256; ++i)
                {
                    finite = finite && std::isfinite (left[i]) && std::isfinite (right[i]);
                    if(checkWoodwind && block<8)signature.push_back((int)std::lround(left[i]*1000000));
                    energy += (double) left[i] * left[i] + (double) right[i] * right[i];
                    peak = std::max (peak, std::max (std::abs (left[i]), std::abs (right[i])));
                }
            }

            if(checkWoodwind)check(woodwindSignatures.insert(signature).second,"woodwind recipes produce identical audio",preset.name);
            if(preset.name=="BA - Stacked Foundation Ref")check(energy/(2*256*140)>1.0e-4,"reference bass VCA loses sustained body",preset.name);
            check (finite, "preset produced non-finite audio", preset.name);
            check (energy > 1.0e-5, "preset is silent", preset.name);
            check (peak < 1.05f, "preset clips", preset.name);
        }

        if(definition.id=="continuum")check(woodwindSignatures.size()==4,"missing woodwind comparison cases",definition.id);
        std::printf ("%s: %d presets, %zu parameters\n", definition.id.c_str(),
                     (int) presets.size(), definition.params.size());
    }

    if (failures == 0)
    {
        std::printf ("All factory preset checks passed.\n");
        return 0;
    }

    return 1;
}
