#include "PedalComponent.h"

#include "HandDrawnLookAndFeel.h"

namespace fpb
{

PedalComponent::PedalComponent (PedalChain& chain, int slotIndex, int pedalIndex,
                                juce::AudioProcessorValueTreeState& apvts)
    : chain_ (chain),
      slotIndex_ (slotIndex),
      pedalIndex_ (pedalIndex),
      info_ (chain.getPedal (pedalIndex).getAlgorithmCredit())
{
    auto& pedal = chain_.getPedal (pedalIndex_);

    title_.setText (pedal.getDisplayName(), juce::dontSendNotification);
    title_.setJustificationType (juce::Justification::centredLeft);
    title_.setFont (HandDrawnLookAndFeel::handFont (18.0f, true));
    title_.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (title_);

    addAndMakeVisible (info_);

    body_ = pedal.createEditor (apvts);

    if (body_ != nullptr)
        addAndMakeVisible (*body_);

    footswitch_.setComponentID (HandDrawnLookAndFeel::kFootswitchId);
    footswitch_.setClickingTogglesState (true);
    footswitch_.setTitle (pedal.getDisplayName() + " on/off");
    footswitch_.setTooltip ("Stomp to bypass " + pedal.getDisplayName().toLowerCase());
    footswitch_.onClick = [this]
    {
        chain_.setBypassed (slotIndex_, ! footswitch_.getToggleState());
    };
    addAndMakeVisible (footswitch_);

    setTitle (pedal.getDisplayName() + " pedal");
    refresh();
}

void PedalComponent::refresh()
{
    const auto slot = chain_.getSlot (slotIndex_);
    footswitch_.setToggleState (! slot.bypassed, juce::dontSendNotification);
}

void PedalComponent::paint (juce::Graphics& g)
{
    const auto seed = HandDrawnLookAndFeel::seedFor (*this);
    const auto bounds = getLocalBounds().toFloat().reduced (2.0f);

    g.setColour (HandDrawnLookAndFeel::paper());
    g.fillRect (bounds);

    g.setColour (HandDrawnLookAndFeel::ink());
    g.strokePath (HandDrawnLookAndFeel::wobblyRect (bounds, seed, 1.6f), juce::PathStrokeType (2.2f));

    // Four little screws, because every pedal has them.
    for (const auto corner : { bounds.getTopLeft(), bounds.getTopRight(), bounds.getBottomLeft(), bounds.getBottomRight() })
    {
        const auto screw = juce::Rectangle<float> (7.0f, 7.0f).withCentre (
            { corner.x < bounds.getCentreX() ? corner.x + 9.0f : corner.x - 9.0f,
              corner.y < bounds.getCentreY() ? corner.y + 9.0f : corner.y - 9.0f });
        g.strokePath (HandDrawnLookAndFeel::wobblyEllipse (screw, seed + (int) corner.x, 0.5f), juce::PathStrokeType (1.0f));
    }

    // A wobbly rule under the title.
    const float y = (float) kTitleHeight - 2.0f;
    g.strokePath (HandDrawnLookAndFeel::wobblyLine ({ bounds.getX() + 10.0f, y }, { bounds.getRight() - 10.0f, y }, seed + 3),
                  juce::PathStrokeType (1.2f));
}

void PedalComponent::resized()
{
    auto area = getLocalBounds().reduced (6);

    auto titleRow = area.removeFromTop (kTitleHeight - 8);
    info_.setBounds (titleRow.removeFromRight (20).withSizeKeepingCentre (20, 20));
    title_.setBounds (titleRow.reduced (4, 0));

    area.removeFromTop (6);

    auto footRow = area.removeFromBottom (kFootswitchHeight);
    footswitch_.setBounds (footRow.withSizeKeepingCentre (kFootswitchHeight, kFootswitchHeight));

    if (body_ != nullptr)
        body_->setBounds (area);
}

} // namespace fpb
