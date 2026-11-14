#include "BoardComponent.h"

#include "HandDrawnLookAndFeel.h"
#include "core/ParameterIds.h"

namespace fpb
{

namespace
{
    constexpr std::array<float, 5> kScales { 0.75f, 1.0f, 1.25f, 1.5f, 2.0f };
    const juce::StringArray kScaleNames { "75%", "100%", "125%", "150%", "200%" };

    constexpr int kMargin = 20;
    constexpr int kTopStripHeight = 80;
    constexpr int kSlotWidth = 170;
    constexpr int kSlotGap = 8;
    constexpr int kSlotHeight = 350;
} // namespace

BoardComponent::BoardComponent (PedalChain& chain, juce::AudioProcessorValueTreeState& apvts)
    : chain_ (chain),
      apvts_ (apvts)
{
    for (int i = 0; i < PedalChain::kMaxSlots; ++i)
    {
        slots_.push_back (std::make_unique<SlotComponent> (chain_, i, apvts_));
        addAndMakeVisible (*slots_.back());
    }

    inputGain_.setTitle ("Input gain");
    inputGain_.setPopupDisplayEnabled (true, false, this);
    inputAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts_, ParamIds::inputGain, inputGain_);
    addAndMakeVisible (inputGain_);

    outputGain_.setTitle ("Output gain");
    outputGain_.setPopupDisplayEnabled (true, false, this);
    outputAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts_, ParamIds::outputGain, outputGain_);
    addAndMakeVisible (outputGain_);

    for (auto* label : { &inputLabel_, &outputLabel_, &scaleLabel_ })
    {
        label->setJustificationType (juce::Justification::centred);
        label->setInterceptsMouseClicks (false, false);
        addAndMakeVisible (*label);
    }

    inputLabel_.setText ("INPUT", juce::dontSendNotification);
    outputLabel_.setText ("OUTPUT", juce::dontSendNotification);
    scaleLabel_.setText ("UI SCALING", juce::dontSendNotification);

    scaleBox_.addItemList (kScaleNames, 1);
    scaleBox_.setTitle ("UI scaling");
    scaleBox_.setSelectedId (2, juce::dontSendNotification);
    scaleBox_.onChange = [this]
    {
        const int index = scaleBox_.getSelectedItemIndex();

        if (juce::isPositiveAndBelow (index, (int) kScales.size()) && onScaleChanged != nullptr)
            onScaleChanged (kScales[(size_t) index]);
    };
    addAndMakeVisible (scaleBox_);

    chain_.addChangeListener (this);
    setSize (kWidth, kHeight);
}

BoardComponent::~BoardComponent()
{
    chain_.removeChangeListener (this);
}

void BoardComponent::setScaleSelection (float scale)
{
    for (size_t i = 0; i < kScales.size(); ++i)
    {
        if (juce::approximatelyEqual (kScales[i], scale))
        {
            scaleBox_.setSelectedItemIndex ((int) i, juce::dontSendNotification);
            return;
        }
    }
}

void BoardComponent::changeListenerCallback (juce::ChangeBroadcaster*)
{
    refreshSlots();
}

void BoardComponent::refreshSlots()
{
    for (auto& slot : slots_)
        slot->refresh();
}

void BoardComponent::paint (juce::Graphics& g)
{
    g.fillAll (HandDrawnLookAndFeel::paper());
    g.setColour (HandDrawnLookAndFeel::ink());

    // Title with a wobbly underline.
    g.setFont (HandDrawnLookAndFeel::handFont (44.0f, true));
    g.drawText ("FOSS PEDALBOARD", titleArea_, juce::Justification::centred, false);

    const float underlineY = (float) titleArea_.getBottom() - 10.0f;
    g.strokePath (HandDrawnLookAndFeel::wobblyLine ({ (float) titleArea_.getX() + 120.0f, underlineY },
                                                    { (float) titleArea_.getRight() - 120.0f, underlineY }, 21, 1.5f),
                  juce::PathStrokeType (2.0f));

    // Signal flow arrow beneath the slots.
    const float arrowY = (float) slotsArea_.getBottom() + 20.0f;
    const juce::Line<float> flow ((float) slotsArea_.getX() + 30.0f, arrowY, (float) slotsArea_.getRight() - 30.0f, arrowY);
    g.drawArrow (flow, 1.8f, 12.0f, 12.0f);

    g.setFont (HandDrawnLookAndFeel::handFont (13.0f, true));
    g.drawText ("IN", slotsArea_.getX(), (int) arrowY - 8, 28, 16, juce::Justification::centredLeft, false);
    g.drawText ("OUT", slotsArea_.getRight() - 28, (int) arrowY - 8, 28, 16, juce::Justification::centredRight, false);

    g.setFont (HandDrawnLookAndFeel::handFont (12.0f));
    g.setColour (HandDrawnLookAndFeel::pencil());
    g.drawText ("AGPLv3  -  hover the (i) on a pedal to see where its sound comes from",
                getLocalBounds().removeFromBottom (24), juce::Justification::centred, false);
}

void BoardComponent::resized()
{
    auto area = getLocalBounds().reduced (kMargin, 0);
    auto top = area.removeFromTop (kTopStripHeight);

    // Input knob + label, top-left
    auto inputArea = top.removeFromLeft (64);
    inputLabel_.setBounds (inputArea.removeFromBottom (16));
    inputGain_.setBounds (inputArea.reduced (0, 8));

    top.removeFromLeft (16);

    // UI scaling box, next to the input knob (as in the reference drawing)
    auto scaleArea = top.removeFromLeft (84);
    scaleLabel_.setBounds (scaleArea.removeFromTop (34).withTrimmedTop (16));
    scaleBox_.setBounds (scaleArea.removeFromTop (24));

    // Output knob + label, top-right
    auto outputArea = top.removeFromRight (64);
    outputLabel_.setBounds (outputArea.removeFromBottom (16));
    outputGain_.setBounds (outputArea.reduced (0, 8));

    titleArea_ = top;

    area.removeFromTop (12);
    slotsArea_ = area.removeFromTop (kSlotHeight);

    auto row = slotsArea_;

    for (auto& slot : slots_)
    {
        slot->setBounds (row.removeFromLeft (kSlotWidth));
        row.removeFromLeft (kSlotGap);
    }
}

} // namespace fpb
