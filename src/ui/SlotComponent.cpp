#include "SlotComponent.h"

#include "HandDrawnLookAndFeel.h"

namespace fpb
{

SlotComponent::SlotComponent (PedalChain& chain, int slotIndex, juce::AudioProcessorValueTreeState& apvts)
    : chain_ (chain),
      slotIndex_ (slotIndex),
      apvts_ (apvts)
{
    const auto slotName = "slot " + juce::String (slotIndex_ + 1);

    moveLeft_.setTitle ("Move " + slotName + " left");
    moveLeft_.setTooltip ("Swap with the pedal on the left");
    moveLeft_.onClick = [this] { chain_.swapSlots (slotIndex_, slotIndex_ - 1); };

    moveRight_.setTitle ("Move " + slotName + " right");
    moveRight_.setTooltip ("Swap with the pedal on the right");
    moveRight_.onClick = [this] { chain_.swapSlots (slotIndex_, slotIndex_ + 1); };

    replace_.setTitle ("Replace pedal in " + slotName);
    replace_.setTooltip ("Swap this pedal for a different one");
    replace_.onClick = [this] { showPedalMenu (replace_); };

    remove_.setTitle ("Remove pedal from " + slotName);
    remove_.setTooltip ("Take this pedal off the board");
    remove_.onClick = [this] { chain_.clearSlot (slotIndex_); };

    add_.setTitle ("Add pedal to " + slotName);
    add_.setTooltip ("Put a pedal in this empty slot");
    add_.onClick = [this] { showPedalMenu (add_); };

    for (auto* button : { &moveLeft_, &replace_, &remove_, &moveRight_, &add_ })
        addChildComponent (*button);

    setTitle ("Pedalboard " + slotName);
    refresh();
}

void SlotComponent::refresh()
{
    const auto slot = chain_.getSlot (slotIndex_);
    const bool occupied = slot.pedalIndex != PedalChain::kNoPedal;

    if (! occupied)
    {
        pedal_.reset();
    }
    else if (pedal_ == nullptr || pedal_->getPedalIndex() != slot.pedalIndex)
    {
        pedal_ = std::make_unique<PedalComponent> (chain_, slotIndex_, slot.pedalIndex, apvts_);
        addAndMakeVisible (*pedal_);
    }
    else
    {
        pedal_->refresh();
    }

    add_.setVisible (! occupied);

    for (auto* button : { &moveLeft_, &replace_, &remove_, &moveRight_ })
        button->setVisible (occupied);

    moveLeft_.setEnabled (slotIndex_ > 0);
    moveRight_.setEnabled (slotIndex_ < PedalChain::kMaxSlots - 1);

    resized();
    repaint();
}

void SlotComponent::showPedalMenu (juce::Component& anchor)
{
    juce::PopupMenu menu;

    for (int i = 0; i < chain_.getNumPedalTypes(); ++i)
    {
        const int inSlot = chain_.findSlotOf (i);
        const bool here = inSlot == slotIndex_;
        const bool elsewhere = inSlot != PedalChain::kNoPedal && ! here;

        auto name = chain_.getPedal (i).getDisplayName();

        if (elsewhere)
            name << "   (in slot " << (inSlot + 1) << ")";

        menu.addItem (i + 1, name, ! elsewhere, here);
    }

    juce::Component::SafePointer<SlotComponent> safeThis (this);

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (anchor),
                        [safeThis] (int result)
                        {
                            if (safeThis == nullptr || result <= 0)
                                return;

                            safeThis->chain_.setPedal (safeThis->slotIndex_, result - 1);
                        });
}

void SlotComponent::paint (juce::Graphics& g)
{
    if (pedal_ != nullptr)
        return;

    // Empty slot: a faint dashed outline and a label.
    const auto seed = HandDrawnLookAndFeel::seedFor (*this);
    auto area = getLocalBounds().toFloat();
    area.removeFromTop ((float) kButtonRowHeight + 4.0f);
    area = area.reduced (4.0f);

    const auto outline = HandDrawnLookAndFeel::wobblyRect (area, seed, 1.0f);
    juce::Path dashed;
    const float dashes[] = { 6.0f, 5.0f };
    juce::PathStrokeType (1.2f).createDashedStroke (dashed, outline, dashes, 2);

    g.setColour (HandDrawnLookAndFeel::pencil());
    g.fillPath (dashed);

    g.setFont (HandDrawnLookAndFeel::handFont (14.0f, true));
    g.drawText ("EMPTY", area.toNearestInt().withTrimmedBottom (area.getHeight() > 120 ? 60 : 0),
                juce::Justification::centred, false);
}

void SlotComponent::resized()
{
    auto area = getLocalBounds();
    auto row = area.removeFromTop (kButtonRowHeight);

    constexpr int gap = 5;
    const int widths[] = { 28, 58, 28, 28 };
    const int total = widths[0] + widths[1] + widths[2] + widths[3] + gap * 3;
    row = row.withSizeKeepingCentre (total, kButtonRowHeight);

    moveLeft_.setBounds (row.removeFromLeft (widths[0]));
    row.removeFromLeft (gap);
    replace_.setBounds (row.removeFromLeft (widths[1]));
    row.removeFromLeft (gap);
    remove_.setBounds (row.removeFromLeft (widths[2]));
    row.removeFromLeft (gap);
    moveRight_.setBounds (row.removeFromLeft (widths[3]));

    area.removeFromTop (4);

    if (pedal_ != nullptr)
        pedal_->setBounds (area);

    add_.setBounds (area.withSizeKeepingCentre (juce::jmin (130, area.getWidth() - 20), 34));
}

} // namespace fpb
