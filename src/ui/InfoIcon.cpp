#include "InfoIcon.h"

#include "HandDrawnLookAndFeel.h"

namespace fpb
{

InfoIcon::InfoIcon (const juce::String& credit)
{
    setTooltip (credit);
    setTitle ("Algorithm info");
    setDescription (credit);
    setHelpText (credit);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setRepaintsOnMouseActivity (true);
}

void InfoIcon::paint (juce::Graphics& g)
{
    const auto seed = HandDrawnLookAndFeel::seedFor (*this);
    const auto bounds = getLocalBounds().toFloat().reduced (1.5f);
    const float d = juce::jmin (bounds.getWidth(), bounds.getHeight());
    const auto circle = bounds.withSizeKeepingCentre (d, d);

    g.setColour (hovered_ ? HandDrawnLookAndFeel::pencil().withAlpha (0.25f) : HandDrawnLookAndFeel::paper());
    g.fillEllipse (circle);

    g.setColour (HandDrawnLookAndFeel::ink());
    g.strokePath (HandDrawnLookAndFeel::wobblyEllipse (circle, seed, 0.8f), juce::PathStrokeType (1.4f));

    g.setFont (HandDrawnLookAndFeel::handFont (d * 0.72f, true));
    g.drawText ("i", circle.toNearestInt(), juce::Justification::centred, false);
}

void InfoIcon::mouseEnter (const juce::MouseEvent&)
{
    hovered_ = true;
    repaint();
}

void InfoIcon::mouseExit (const juce::MouseEvent&)
{
    hovered_ = false;
    repaint();
}

} // namespace fpb
