#include "ui/PanelBase.h"

#include <algorithm>
#include <cmath>

namespace triad::ui
{

PanelBase::PanelBase (EditorHost& host, juce::String title, int designWidth, int designHeight,
                      juce::Colour accent)
    : host_ (host), title_ (std::move (title)), designWidth_ (designWidth),
      designHeight_ (designHeight), accent_ (accent)
{
    const auto id = juce::String (host_.definition().id);
    finish_ = id == "polybrute" ? Finish::PolyBrute
              : (id == "moogone" || id == "one" ? Finish::MoogOne : Finish::Matriarch);

    setSize (designWidth_, designHeight_);
    setOpaque (true);
    fallbackParameter_ = std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "panel.fallback", 1 }, "Fallback",
        juce::NormalisableRange<float> (0.0f, 3.0f, 1.0f), 0.0f);
    refreshPatchCables();
}

PanelBase::~PanelBase() = default;

void PanelBase::setBranding (juce::String mainText, juce::String subText)
{
    brandMain_ = std::move (mainText);
    brandSub_ = std::move (subText);
}

juce::RangedAudioParameter* PanelBase::parameterFor (const juce::String& paramId) const
{
    return host_.apvts().getParameter (paramId);
}

void PanelBase::addModule (const juce::String& moduleTitle, juce::Rectangle<int> area,
                           ModuleTone tone)
{
    addModule (moduleTitle, {}, area, tone);
}

void PanelBase::addModule (const juce::String& moduleTitle, const juce::String& subtitle,
                           juce::Rectangle<int> area, ModuleTone tone)
{
    modules_.push_back ({ moduleTitle, subtitle, area, tone });
}

void PanelBase::addLabel (const juce::String& text, juce::Rectangle<int> area, float fontSize,
                          juce::Colour colour, bool bold)
{
    auto label = std::make_unique<juce::Label>();
    label->setText (text, juce::dontSendNotification);
    label->setJustificationType (juce::Justification::centred);
    label->setInterceptsMouseClicks (false, false);
    label->setColour (juce::Label::textColourId, colour);
    label->setFont (juce::Font (juce::FontOptions (fontSize, bold ? juce::Font::bold : juce::Font::plain)));
    label->setMinimumHorizontalScale (0.55f);
    label->setBounds (area);
    addAndMakeVisible (*label);
    labels_.push_back (std::move (label));
}

juce::Label& PanelBase::addCaption (const juce::String& text, juce::Rectangle<int> area,
                                    juce::Justification justification, float fontSize,
                                    juce::Colour colour, bool bold)
{
    auto label = std::make_unique<juce::Label>();
    label->setText (text, juce::dontSendNotification);
    label->setJustificationType (justification);
    label->setInterceptsMouseClicks (false, false);
    label->setColour (juce::Label::textColourId, colour);
    label->setFont (juce::Font (juce::FontOptions (fontSize, bold ? juce::Font::bold : juce::Font::plain)));
    label->setMinimumHorizontalScale (0.55f);
    label->setBounds (area);
    addAndMakeVisible (*label);
    auto& ref = *label;
    labels_.push_back (std::move (label));
    return ref;
}

juce::Slider& PanelBase::addKnob (const juce::String& paramId, const juce::String& labelText,
                                  juce::Rectangle<int> area)
{
    auto spec = definition().find (paramId.toStdString());
    auto* parameter = parameterFor (paramId);

    auto slider = std::make_unique<juce::Slider>();
    slider->setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider->setScrollWheelEnabled (true);
    slider->setMouseDragSensitivity (110);
    slider->setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                 juce::MathConstants<float>::pi * 2.8f, true);
    slider->setTooltip (spec != nullptr ? juce::String (spec->name) + "  [" + paramId + "]" : paramId);

    const auto labelHeight = 12;
    const auto size = juce::jmax (18, juce::jmin (area.getWidth(), area.getHeight() - labelHeight));
    slider->setBounds (area.getX() + (area.getWidth() - size) / 2, area.getY(), size, size);

    if (spec != nullptr)
        slider->setDoubleClickReturnValue (true, juce::jlimit (spec->min, spec->max, spec->def));

    addAndMakeVisible (*slider);

    if (parameter != nullptr)
        sliderAttachments_.push_back (
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (host_.apvts(), paramId, *slider));

    auto& ref = *slider;
    sliders_.push_back (std::move (slider));

    addLabel (labelText, { area.getX(), area.getBottom() - labelHeight, area.getWidth(), labelHeight },
              10.2f, juce::Colour (0xffd3cfc6), false);

    return ref;
}

juce::Slider& PanelBase::addKnob (const juce::String& paramId, const juce::String& labelText,
                                  juce::Point<int> centre, int diameter)
{
    return addKnob (paramId, labelText,
                    { centre.x - diameter / 2, centre.y - diameter / 2,
                      diameter, diameter + 12 });
}

juce::Slider& PanelBase::addFader (const juce::String& paramId, const juce::String& labelText,
                                  juce::Rectangle<int> area)
{
    auto spec = definition().find (paramId.toStdString());
    auto* parameter = parameterFor (paramId);

    auto slider = std::make_unique<juce::Slider>();
    slider->setSliderStyle (juce::Slider::LinearVertical);
    slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider->setTooltip (spec != nullptr ? juce::String (spec->name) + "  [" + paramId + "]" : paramId);

    const auto labelHeight = 11;
    const auto faderHeight = juce::jmax (24, area.getHeight() - labelHeight);
    slider->setBounds (area.getX() + 4, area.getY(), area.getWidth() - 8, faderHeight);

    if (spec != nullptr)
        slider->setDoubleClickReturnValue (true, juce::jlimit (spec->min, spec->max, spec->def));

    addAndMakeVisible (*slider);

    if (parameter != nullptr)
        sliderAttachments_.push_back (
            std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (host_.apvts(), paramId, *slider));

    auto& ref = *slider;
    sliders_.push_back (std::move (slider));

    addLabel (labelText, { area.getX(), area.getBottom() - labelHeight, area.getWidth(), labelHeight },
              9.6f, juce::Colour (0xffe4e0d7), false);

    return ref;
}

juce::ComboBox& PanelBase::addChoice (const juce::String& paramId, const juce::String& labelText,
                                      juce::Rectangle<int> area)
{
    auto spec = definition().find (paramId.toStdString());
    auto* parameter = parameterFor (paramId);

    auto combo = std::make_unique<juce::ComboBox>();
    juce::StringArray items;

    if (parameter != nullptr)
        items = parameter->getAllValueStrings();

    if (items.isEmpty() && spec != nullptr)
        for (const auto& choice : spec->choices)
            items.add (choice);

    combo->addItemList (items, 1);
    combo->setJustificationType (juce::Justification::centredLeft);
    combo->setTooltip (spec != nullptr ? juce::String (spec->name) + "  [" + paramId + "]" : paramId);

    const auto labelHeight = 11;
    const auto comboHeight = juce::jlimit (18, 24, area.getHeight() - labelHeight);
    combo->setBounds (area.getX(), area.getY(), area.getWidth(), comboHeight);

    if (parameter != nullptr)
        comboAttachments_.push_back (
            std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (host_.apvts(), paramId, *combo));

    addAndMakeVisible (*combo);
    auto& ref = *combo;
    combos_.push_back (std::move (combo));

    if (labelText.isNotEmpty())
        addLabel (labelText, { area.getX(), area.getBottom() - labelHeight, area.getWidth(), labelHeight },
                  9.6f, juce::Colour (0xffe4e0d7), false);

    return ref;
}

HardwareSwitch& PanelBase::addSwitch (const juce::String& paramId, const juce::String& labelText,
                                      juce::Rectangle<int> area, bool vertical)
{
    auto* parameter = parameterFor (paramId);
    auto spec = definition().find (paramId.toStdString());

    if (parameter == nullptr)
    {
        jassertfalse;
        parameter = fallbackParameter_.get();
    }

    auto component = std::make_unique<HardwareSwitch> (
        *parameter, vertical ? HardwareSwitch::Orientation::Vertical : HardwareSwitch::Orientation::Horizontal);
    component->setTooltip (spec != nullptr ? juce::String (spec->name) + "  [" + paramId + "]" : paramId);

    const auto labelHeight = 11;
    const auto switchHeight = juce::jmax (18, area.getHeight() - labelHeight);
    component->setBounds (area.getX(), area.getY(), area.getWidth(), switchHeight);

    addAndMakeVisible (*component);
    auto& ref = *component;
    switches_.push_back (std::move (component));

    if (labelText.isNotEmpty())
        addLabel (labelText, { area.getX(), area.getBottom() - labelHeight, area.getWidth(), labelHeight },
                  9.6f, juce::Colour (0xffe4e0d7), false);

    return ref;
}

LedButton& PanelBase::addLed (const juce::String& paramId, const juce::String& labelText,
                              juce::Rectangle<int> area)
{
    auto* parameter = parameterFor (paramId);
    auto spec = definition().find (paramId.toStdString());

    if (parameter == nullptr)
    {
        jassertfalse;
        parameter = fallbackParameter_.get();
    }

    auto component = std::make_unique<LedButton> (*parameter, labelText);
    component->setAccent (accent_);
    component->setTooltip (spec != nullptr ? juce::String (spec->name) + "  [" + paramId + "]" : paramId);
    component->setBounds (area);
    addAndMakeVisible (*component);
    auto& ref = *component;
    leds_.push_back (std::move (component));
    return ref;
}

Jack& PanelBase::addJack (const juce::String& patchId, juce::Point<int> centre, int diameter)
{
    return addJack (patchId, { centre.x - diameter / 2, centre.y - diameter / 2, diameter, diameter });
}

Jack& PanelBase::addJack (const juce::String& patchId, juce::Rectangle<int> area)
{
    const auto index = definition().patchIndexOf (patchId.toStdString());
    const auto* spec = index >= 0 ? &definition().patchPoints[(size_t) index] : nullptr;

    auto jack = std::make_unique<Jack> (index, spec != nullptr && spec->isOutput,
                                        spec != nullptr ? juce::String (spec->name) : patchId,
                                        accent_);
    jack->setBounds (area);
    jack->onJackClicked = [this] (Jack& j) { handleJackClicked (j); };
    jack->onJackRightClicked = [this] (Jack& j) { handleJackRightClicked (j); };
    addAndMakeVisible (*jack);
    auto& ref = *jack;
    jacks_.push_back (std::move (jack));

    if (spec == nullptr)
    {
        ref.setTooltip ("Missing patch point: " + patchId);
    }
    else
    {
        const auto labelHeight = 8;
        addLabel (juce::String (spec->name),
                  { area.getX() - 22, area.getBottom(), area.getWidth() + 44, labelHeight },
                  8.0f, juce::Colour (0xffdedad1), false);
    }

    return ref;
}

juce::TextButton& PanelBase::addButton (const juce::String& text, juce::Rectangle<int> area,
                                       std::function<void()> onClick)
{
    auto button = std::make_unique<juce::TextButton> (text);
    button->setBounds (area);
    button->setTooltip (text);
    if (onClick != nullptr)
        button->onClick = std::move (onClick);

    addAndMakeVisible (*button);
    auto& ref = *button;
    buttons_.push_back (std::move (button));
    return ref;
}

SourceMatrix& PanelBase::addSourceMatrix (std::vector<juce::String> sourceParameterIds,
                                         int rows, juce::Rectangle<int> area)
{
    auto matrix = std::make_unique<SourceMatrix> (host_.apvts(), std::move (sourceParameterIds),
                                                   rows, 8);
    matrix->setBounds (area);
    addAndMakeVisible (*matrix);
    auto& ref = *matrix;
    matrices_.push_back (std::move (matrix));
    return ref;
}

LcdDisplay& PanelBase::addLcd (juce::Rectangle<int> area)
{
    auto lcd = std::make_unique<LcdDisplay>();
    lcd->setBounds (area);
    addAndMakeVisible (*lcd);
    auto& ref = *lcd;
    lcds_.push_back (std::move (lcd));
    return ref;
}

juce::Slider& PanelBase::knobFor (const juce::String& paramId)
{
    for (size_t i = 0; i < sliders_.size(); ++i)
        if (sliders_[i]->getTooltip().contains (paramId))
            return *sliders_[i];

    jassertfalse;
    return *sliders_.front();
}

void PanelBase::clearControls()
{
    sliderAttachments_.clear();
    comboAttachments_.clear();
    labels_.clear();
    sliders_.clear();
    combos_.clear();
    switches_.clear();
    leds_.clear();
    lcds_.clear();
    buttons_.clear();
    matrices_.clear();
    jacks_.clear();
    modules_.clear();
    selectedJack_ = -1;
}

void PanelBase::pollPatchCables()
{
    const auto current = host_.patchCables();

    if (current.size() == cables_.size())
    {
        bool equal = true;
        for (size_t i = 0; i < current.size(); ++i)
        {
            if (current[i].from != cables_[i].from || current[i].to != cables_[i].to)
            {
                equal = false;
                break;
            }
        }

        if (equal)
            return;
    }

    refreshPatchCables();
}

void PanelBase::refreshPatchCables()
{
    cables_ = host_.patchCables();

    for (auto& jack : jacks_)
    {
        int count = 0;
        for (const auto& cable : cables_)
            if (cable.from == jack->patchIndex() || cable.to == jack->patchIndex())
                ++count;
        jack->setCableCount (count);
    }

    repaint();
}

Jack* PanelBase::findJack (int patchIndex) const
{
    for (const auto& jack : jacks_)
        if (jack->patchIndex() == patchIndex)
            return jack.get();

    return nullptr;
}

juce::Colour PanelBase::cableColour (int index) const
{
    static const juce::Colour colours[] = {
        juce::Colour (0xffd64b3a), juce::Colour (0xff3f8ed6), juce::Colour (0xffd8b03c),
        juce::Colour (0xff57b06a), juce::Colour (0xffb05fce), juce::Colour (0xffd07a3a)
    };

    return colours[(size_t) std::abs (index) % 6];
}

void PanelBase::handleJackClicked (Jack& jack)
{
    if (jack.patchIndex() < 0)
        return;

    if (selectedJack_ < 0)
    {
        selectedJack_ = jack.patchIndex();
        jack.setSelected (true);
        return;
    }

    if (selectedJack_ == jack.patchIndex())
    {
        jack.setSelected (false);
        selectedJack_ = -1;
        return;
    }

    auto* first = findJack (selectedJack_);
    if (first == nullptr)
    {
        selectedJack_ = jack.patchIndex();
        jack.setSelected (true);
        return;
    }

    int from = first->patchIndex();
    int to = jack.patchIndex();

    if (first->isOutput() && ! jack.isOutput())
    {
        from = first->patchIndex();
        to = jack.patchIndex();
    }
    else if (! first->isOutput() && jack.isOutput())
    {
        from = jack.patchIndex();
        to = first->patchIndex();
    }

    // Only one cable may drive a destination.
    for (const auto& cable : cables_)
        if (cable.to == to && ! (cable.from == from && cable.to == to))
            host_.setPatchCable (cable.from, cable.to, false);

    host_.setPatchCable (from, to, true);

    first->setSelected (false);
    selectedJack_ = -1;
    refreshPatchCables();

    if (onPatchChanged)
        onPatchChanged();
}

void PanelBase::handleJackRightClicked (Jack& jack)
{
    auto list = host_.patchCables();

    for (const auto& cable : list)
        if (cable.from == jack.patchIndex() || cable.to == jack.patchIndex())
            host_.setPatchCable (cable.from, cable.to, false);

    if (selectedJack_ == jack.patchIndex())
        selectedJack_ = -1;

    jack.setSelected (false);
    refreshPatchCables();

    if (onPatchChanged)
        onPatchChanged();
}

namespace
{
void drawWood (juce::Graphics& g, juce::Rectangle<float> area, bool vertical, int seed)
{
    juce::ColourGradient wood (juce::Colour (0xff7a4d2c), area.getX(), area.getY(),
                               juce::Colour (0xff3b2415), area.getRight(), area.getBottom(), false);
    wood.addColour (0.35, juce::Colour (0xff5f3a21));
    wood.addColour (0.7, juce::Colour (0xff6d4529));
    g.setGradientFill (wood);
    g.fillRect (area);

    juce::Random random (seed);
    for (int i = 0; i < 18; ++i)
    {
        const auto offset = random.nextFloat();
        const auto thickness = 0.5f + random.nextFloat() * 1.2f;
        g.setColour (juce::Colours::black.withAlpha (0.06f + random.nextFloat() * 0.1f));

        if (vertical)
        {
            const auto x = area.getX() + offset * area.getWidth();
            g.drawVerticalLine ((int) x, area.getY(), area.getBottom());
        }
        else
        {
            const auto y = area.getY() + offset * area.getHeight();
            g.drawHorizontalLine ((int) y, area.getX(), area.getRight());
        }

        juce::ignoreUnused (thickness);
    }

    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawRect (area, 1.0f);
}

void drawScrew (juce::Graphics& g, juce::Point<float> centre, bool dark)
{
    const auto radius = 3.4f;
    const auto body = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);

    if (dark)
    {
        juce::ColourGradient metal (juce::Colour (0xff5a5a60), body.getX(), body.getY(),
                                     juce::Colour (0xff111113), body.getRight(), body.getBottom(), false);
        g.setGradientFill (metal);
        g.fillEllipse (body);
        g.setColour (juce::Colours::black.withAlpha (0.7f));
        g.drawLine (centre.x - 2.2f, centre.y - 2.2f, centre.x + 2.2f, centre.y + 2.2f, 1.0f);
    }
    else
    {
        juce::ColourGradient metal (juce::Colour (0xffd8d5cd), body.getX(), body.getY(),
                                     juce::Colour (0xff55524c), body.getRight(), body.getBottom(), false);
        g.setGradientFill (metal);
        g.fillEllipse (body);
        g.setColour (juce::Colour (0xff2a2a2a).withAlpha (0.8f));
        g.drawLine (centre.x - 2.2f, centre.y, centre.x + 2.2f, centre.y, 1.0f);
        g.drawLine (centre.x, centre.y - 2.2f, centre.x, centre.y + 2.2f, 1.0f);
    }

    g.setColour (juce::Colours::white.withAlpha (0.14f));
    g.drawEllipse (body.reduced (2.2f), 0.6f);
}
} // namespace

void PanelBase::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    // ---- Panel base -------------------------------------------------------
    if (finish_ == Finish::Matriarch)
    {
        g.setColour (juce::Colour (0xff0a0a0b));
        g.fillRect (bounds);

        for (float y = 0.0f; y < bounds.getHeight(); y += 2.0f)
        {
            const auto alpha = 0.012f + 0.012f * std::sin (y * 0.13f);
            g.setColour (juce::Colours::white.withAlpha (alpha));
            g.drawHorizontalLine ((int) y, 0.0f, bounds.getWidth());
        }

        // Black wooden end cheeks.
        for (const auto& side : { juce::Rectangle<float> (0.0f, 0.0f, 6.0f, bounds.getHeight()),
                                  juce::Rectangle<float> (bounds.getRight() - 6.0f, 0.0f, 6.0f,
                                                          bounds.getHeight()) })
        {
            g.setColour (juce::Colour (0xff040405));
            g.fillRect (side);
            g.setColour (juce::Colour (0xff1b1b1d).withAlpha (0.5f));
            for (float x = side.getX() + 1.0f; x < side.getRight(); x += 2.0f)
                g.drawVerticalLine ((int) x, 0.0f, bounds.getHeight());
        }
    }
    else if (finish_ == Finish::PolyBrute)
    {
        juce::ColourGradient background (juce::Colour (0xff2c3236), 0.0f, 0.0f,
                                         juce::Colour (0xff1c2124), 0.0f, bounds.getBottom(), false);
        g.setGradientFill (background);
        g.fillRect (bounds);

        for (float y = 0.0f; y < bounds.getHeight(); y += 2.0f)
        {
            g.setColour (juce::Colours::white.withAlpha (0.016f));
            g.drawHorizontalLine ((int) y, 0.0f, bounds.getWidth());
        }

        drawWood (g, { 0.0f, 0.0f, 8.0f, bounds.getHeight() }, true, 101);
        drawWood (g, { bounds.getRight() - 8.0f, 0.0f, 8.0f, bounds.getHeight() }, true, 102);

        juce::ColourGradient rail (juce::Colour (0xffc9c7c0), 0.0f, 0.0f,
                                    juce::Colour (0xff5d5c58), 0.0f, 3.0f, false);
        g.setGradientFill (rail);
        g.fillRect (0.0f, 0.0f, bounds.getWidth(), 3.0f);
    }
    else
    {
        g.setColour (juce::Colour (0xff0d0d0e));
        g.fillRect (bounds);

        // Moog One sits in a wooden cabinet.
        const auto frame = 8.0f;
        drawWood (g, { 0.0f, 0.0f, bounds.getWidth(), frame }, false, 201);
        drawWood (g, { 0.0f, bounds.getBottom() - frame, bounds.getWidth(), frame }, false, 202);
        drawWood (g, { 0.0f, 0.0f, frame, bounds.getHeight() }, true, 203);
        drawWood (g, { bounds.getRight() - frame, 0.0f, frame, bounds.getHeight() }, true, 204);

        g.setColour (juce::Colour (0xffc4bfb2).withAlpha (0.35f));
        g.drawRect (bounds.reduced (frame + 1.0f, frame + 1.0f), 1.0f);

        // Left / console / right panel separators.
        juce::ColourGradient trim (juce::Colour (0xffb9b6ad), 0.0f, 0.0f,
                                    juce::Colour (0xff3c3b38), 0.0f, bounds.getBottom(), false);
        g.setGradientFill (trim);
        g.fillRect (772.0f, 8.0f, 2.0f, bounds.getHeight() - 16.0f);
        g.fillRect (1082.0f, 8.0f, 2.0f, bounds.getHeight() - 16.0f);
    }

    // Top bevel highlight and bottom shadow.
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawHorizontalLine (0, 0.0f, bounds.getWidth());
    g.setColour (juce::Colours::black.withAlpha (0.65f));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, bounds.getWidth());

    // ---- Modules ----------------------------------------------------------
    for (const auto& module : modules_)
    {
        const auto area = module.bounds.toFloat();

        juce::Colour fill = juce::Colour (0xff0e0e0f);
        juce::Colour border = juce::Colour (0xff8a8a84);
        juce::Colour titleColour = juce::Colour (0xfff2efe5);
        juce::Colour subtitleColour = juce::Colour (0xffb8b4aa);

        if (finish_ == Finish::PolyBrute)
        {
            fill = juce::Colour (0xff20262a);
            border = juce::Colour (0xff7a8082);
            titleColour = juce::Colour (0xffeae7df);
            subtitleColour = juce::Colour (0xffb5b2aa);
        }
        else if (finish_ == Finish::MoogOne)
        {
            fill = juce::Colour (0xff101012);
            border = juce::Colour (0xff7c7c82);
            titleColour = juce::Colour (0xffeae5da);
            subtitleColour = juce::Colour (0xffb5b0a6);
        }

        if (module.tone == ModuleTone::Light)
        {
            juce::ColourGradient plate (juce::Colour (0xff727678), area.getX(), area.getY(),
                                         juce::Colour (0xff4c5052), area.getX(), area.getBottom(), false);
            g.setGradientFill (plate);
        }
        else
        {
            g.setColour (fill);
        }
        g.fillRoundedRectangle (area, 2.0f);
        g.setColour (border.withAlpha (0.62f));
        g.drawRoundedRectangle (area.reduced (0.5f), 2.0f, 1.0f);

        // Engraved plate edge.
        g.setColour (juce::Colours::white.withAlpha (0.035f));
        g.drawHorizontalLine ((int) area.getY() + 1, area.getX() + 3.0f, area.getRight() - 3.0f);

        auto titleFont = juce::Font (juce::FontOptions (12.0f, juce::Font::bold))
                             .withExtraKerningFactor (0.12f);
        g.setFont (titleFont);
        g.setColour (titleColour);

        const auto titleArea = module.bounds.withHeight (16).reduced (5, 0);
        g.drawText (module.title.toUpperCase(), titleArea, juce::Justification::centred);

        if (finish_ == Finish::MoogOne && module.title.isNotEmpty())
        {
            g.setColour (accent_.withAlpha (0.9f));
            g.fillRect (juce::Rectangle<float> (area.getX() + 2.0f, area.getY() + 2.0f, 2.0f, 11.0f));
        }

        if (module.subtitle.isNotEmpty())
        {
            g.setColour (subtitleColour);
            g.setFont (juce::Font (juce::FontOptions (7.5f)));
            g.drawText (module.subtitle, module.bounds.withTrimmedTop (13).withHeight (9).reduced (5, 0),
                        juce::Justification::centred);
        }

        g.setColour (juce::Colours::white.withAlpha (0.09f));
        g.drawHorizontalLine (module.bounds.getY() + 23, area.getX() + 5.0f, area.getRight() - 5.0f);
    }

    // ---- Branding ---------------------------------------------------------
    if (brandMain_.isNotEmpty())
    {
        const auto brandArea = getLocalBounds().removeFromBottom (48).withTrimmedLeft (16);

        if (finish_ == Finish::Matriarch)
        {
            auto font = juce::Font (juce::FontOptions (29.0f, juce::Font::bold))
                            .withExtraKerningFactor (0.05f);
            g.setFont (font);
            g.setColour (juce::Colour (0xfff0ece2));

            juce::Rectangle<int> textArea (brandArea.getX(), brandArea.getY() + 2, 250, 34);
            g.drawText (brandMain_, textArea, juce::Justification::centredLeft);

            g.setColour (juce::Colour (0xffd9d5cb).withAlpha (0.55f));
            g.setFont (juce::Font (juce::FontOptions (7.5f)).withExtraKerningFactor (0.28f));
            g.drawText (brandSub_.toUpperCase(),
                        brandArea.withTrimmedLeft (222).withHeight (48),
                        juce::Justification::centredLeft);

            // Moog script mark at the right.
            const auto logoArea = getLocalBounds().removeFromBottom (44).removeFromRight (110);
            g.setColour (juce::Colour (0xfff0ece2).withAlpha (0.9f));
            g.setFont (juce::Font (juce::FontOptions ("Helvetica Neue", 24.0f, juce::Font::bold | juce::Font::italic)));
            g.drawText ("moog", logoArea, juce::Justification::centred);
        }
        else if (finish_ == Finish::PolyBrute)
        {
            auto font = juce::Font (juce::FontOptions (27.0f, juce::Font::bold))
                            .withExtraKerningFactor (0.18f);
            g.setFont (font);
            g.setColour (juce::Colour (0xffe8e5de));
            g.drawText (brandMain_, brandArea, juce::Justification::centredLeft);

            g.setColour (juce::Colour (0xff8f8c85));
            g.setFont (juce::Font (juce::FontOptions (8.0f)).withExtraKerningFactor (0.4f));
            g.drawText ("ARTURIA  /  " + brandSub_.toUpperCase(),
                        brandArea.withTrimmedTop (26), juce::Justification::centredLeft);

            g.setColour (juce::Colour (0xff3f93e8));
            g.fillRect (juce::Rectangle<float> ((float) brandArea.getX(), (float) brandArea.getY() + 2.0f,
                                                3.0f, 30.0f));
        }
        else
        {
            auto font = juce::Font (juce::FontOptions (26.0f, juce::Font::bold))
                            .withExtraKerningFactor (0.08f);
            g.setFont (font);
            g.setColour (juce::Colour (0xffe9e4d8));
            g.drawText (brandMain_, brandArea, juce::Justification::centredLeft);

            g.setColour (juce::Colour (0xff9a958a));
            g.setFont (juce::Font (juce::FontOptions (7.5f)).withExtraKerningFactor (0.35f));
            g.drawText (brandSub_.toUpperCase(), brandArea.withTrimmedTop (26),
                        juce::Justification::centredLeft);

            g.setColour (juce::Colour (0xffd9a441).withAlpha (0.85f));
            g.fillRect (juce::Rectangle<float> ((float) brandArea.getX(), (float) brandArea.getY() + 2.0f,
                                                3.0f, 30.0f));
        }
    }

    // ---- Screws and panel edge -------------------------------------------
    const auto darkScrews = finish_ == Finish::Matriarch;
    drawScrew (g, { 11.0f, 11.0f }, darkScrews);
    drawScrew (g, { (float) getWidth() - 11.0f, 11.0f }, darkScrews);
    drawScrew (g, { 11.0f, (float) getHeight() - 11.0f }, darkScrews);
    drawScrew (g, { (float) getWidth() - 11.0f, (float) getHeight() - 11.0f }, darkScrews);

    // Wooden cheeks are the outermost layer on the hardware.
    if (finish_ == Finish::PolyBrute)
    {
        drawWood (g, { 0.0f, 0.0f, 12.0f, bounds.getHeight() }, true, 301);
        drawWood (g, { bounds.getRight() - 12.0f, 0.0f, 12.0f, bounds.getHeight() }, true, 302);
    }
    else if (finish_ == Finish::Matriarch)
    {
        for (const auto& side : { juce::Rectangle<float> (0.0f, 0.0f, 7.0f, bounds.getHeight()),
                                  juce::Rectangle<float> (bounds.getRight() - 7.0f, 0.0f, 7.0f,
                                                          bounds.getHeight()) })
        {
            g.setColour (juce::Colour (0xff050506));
            g.fillRect (side);
            g.setColour (juce::Colour (0xff202022));
            g.drawVerticalLine ((int) side.getX(), 0.0f, bounds.getHeight());
        }
    }

    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.drawRect (getLocalBounds(), 2);
}

void PanelBase::paintOverChildren (juce::Graphics& g)
{
    for (const auto& cable : cables_)
    {
        auto* from = findJack (cable.from);
        auto* to = findJack (cable.to);

        if (from == nullptr || to == nullptr)
            continue;

        const auto p1 = from->getBounds().getCentre().toFloat();
        const auto p2 = to->getBounds().getCentre().toFloat();

        const auto distance = p1.getDistanceFrom (p2);
        const auto sag = juce::jlimit (14.0f, 90.0f, distance * 0.18f + 12.0f);

        juce::Path path;
        path.startNewSubPath (p1);
        path.cubicTo ({ p1.x, p1.y + sag }, { p2.x, p2.y + sag }, p2);

        const auto colour = cableColour (cable.from);

        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.strokePath (path, juce::PathStrokeType (6.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        g.setColour (colour.withAlpha (0.9f));
        g.strokePath (path, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        g.setColour (colour.brighter (0.3f).withAlpha (0.45f));
        g.strokePath (path, juce::PathStrokeType (1.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    }

    if (host_.hasPatching() && ! cables_.empty())
    {
        g.setColour (juce::Colour (0xff8f8c85).withAlpha (0.8f));
        g.setFont (juce::Font (juce::FontOptions (8.5f)));
        g.drawText ("PATCH: click an output, then an input. Right-click a jack to clear.",
                    getLocalBounds().removeFromBottom (14).reduced (18, 0),
                    juce::Justification::centredRight);
    }
}

} // namespace triad::ui
