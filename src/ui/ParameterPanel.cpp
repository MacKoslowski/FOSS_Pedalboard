#include "ParameterPanel.h"

namespace fpb
{

ParameterPanel::ParameterPanel (juce::AudioProcessorValueTreeState& apvts, std::vector<Item> items)
{
    for (const auto& item : items)
        addControl (apvts, item);
}

ParameterPanel::~ParameterPanel() = default;

int ParameterPanel::heightForItems (int numItems)
{
    const int rows = (numItems + kColumns - 1) / kColumns;
    return rows * kCellHeight;
}

void ParameterPanel::addControl (juce::AudioProcessorValueTreeState& apvts, const Item& item)
{
    auto* parameter = apvts.getParameter (item.paramId);
    jassert (parameter != nullptr);

    if (parameter == nullptr)
        return;

    Control control;

    if (dynamic_cast<juce::AudioParameterBool*> (parameter) != nullptr)
    {
        auto button = std::make_unique<juce::ToggleButton>();
        button->setTitle (parameter->getName (64));
        control.buttonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, item.paramId, *button);
        control.widget = std::move (button);
    }
    else if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (parameter))
    {
        auto combo = std::make_unique<juce::ComboBox>();
        combo->addItemList (choice->choices, 1);
        combo->setTitle (parameter->getName (64));
        control.comboAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, item.paramId, *combo);
        control.widget = std::move (combo);
    }
    else
    {
        auto slider = std::make_unique<juce::Slider> (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox);
        slider->setTitle (parameter->getName (64));
        slider->setPopupDisplayEnabled (true, false, this);
        control.sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, item.paramId, *slider);
        control.widget = std::move (slider);
    }

    control.label = std::make_unique<juce::Label> (juce::String(), item.label);
    control.label->setJustificationType (juce::Justification::centred);
    control.label->setInterceptsMouseClicks (false, false);
    control.label->setAccessible (false);

    addAndMakeVisible (*control.widget);
    addAndMakeVisible (*control.label);
    controls_.push_back (std::move (control));
}

void ParameterPanel::resized()
{
    const int totalWidth = kColumns * kCellWidth;
    const int left = (getWidth() - totalWidth) / 2;

    for (size_t i = 0; i < controls_.size(); ++i)
    {
        const int column = (int) i % kColumns;
        const int row = (int) i / kColumns;

        juce::Rectangle<int> cell (left + column * kCellWidth, row * kCellHeight, kCellWidth, kCellHeight);
        auto controlArea = cell.removeFromTop (kControlHeight).reduced (4);

        // Toggles and combos look better a little shorter than a knob.
        if (dynamic_cast<juce::Slider*> (controls_[i].widget.get()) == nullptr)
            controlArea = controlArea.withSizeKeepingCentre (controlArea.getWidth(), 24);

        controls_[i].widget->setBounds (controlArea);
        controls_[i].label->setBounds (cell);
    }
}

} // namespace fpb
