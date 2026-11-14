#include "HandDrawnLookAndFeel.h"

#include <cmath>
#include <functional>
#include <vector>

namespace fpb
{

HandDrawnLookAndFeel::HandDrawnLookAndFeel()
{
    const auto inkColour = ink();
    const auto paperColour = paper();

    setColour (juce::ResizableWindow::backgroundColourId, paperColour);
    setColour (juce::DocumentWindow::textColourId, inkColour);

    setColour (juce::Slider::rotarySliderFillColourId, inkColour);
    setColour (juce::Slider::rotarySliderOutlineColourId, inkColour);
    setColour (juce::Slider::thumbColourId, inkColour);
    setColour (juce::Slider::textBoxTextColourId, inkColour);

    setColour (juce::Label::textColourId, inkColour);

    setColour (juce::TextButton::buttonColourId, paperColour);
    setColour (juce::TextButton::buttonOnColourId, pencil().withAlpha (0.4f));
    setColour (juce::TextButton::textColourOffId, inkColour);
    setColour (juce::TextButton::textColourOnId, inkColour);

    setColour (juce::ToggleButton::textColourId, inkColour);
    setColour (juce::ToggleButton::tickColourId, inkColour);
    setColour (juce::ToggleButton::tickDisabledColourId, pencil());

    setColour (juce::ComboBox::backgroundColourId, paperColour);
    setColour (juce::ComboBox::textColourId, inkColour);
    setColour (juce::ComboBox::outlineColourId, inkColour);
    setColour (juce::ComboBox::arrowColourId, inkColour);
    setColour (juce::ComboBox::buttonColourId, paperColour);
    setColour (juce::ComboBox::focusedOutlineColourId, inkColour);

    setColour (juce::PopupMenu::backgroundColourId, paperColour);
    setColour (juce::PopupMenu::textColourId, inkColour);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, inkColour);
    setColour (juce::PopupMenu::highlightedTextColourId, paperColour);

    setColour (juce::TooltipWindow::backgroundColourId, paperColour);
    setColour (juce::TooltipWindow::textColourId, inkColour);
    setColour (juce::TooltipWindow::outlineColourId, inkColour);

    setColour (juce::BubbleComponent::backgroundColourId, paperColour);
    setColour (juce::BubbleComponent::outlineColourId, inkColour);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

juce::Font HandDrawnLookAndFeel::handFont (float height, bool bold)
{
    return juce::Font (juce::FontOptions (height, bold ? juce::Font::bold : juce::Font::plain));
}

int HandDrawnLookAndFeel::seedFor (const juce::Component& component)
{
    return (int) (std::hash<const void*> {}(&component) & 0x7fffffff);
}

juce::Path HandDrawnLookAndFeel::wobblyRect (juce::Rectangle<float> r, int seed, float wobble)
{
    juce::Random random (seed);
    auto jitter = [&random, wobble] { return (random.nextFloat() * 2.0f - 1.0f) * wobble; };

    constexpr float step = 9.0f;
    std::vector<juce::Point<float>> points;

    for (float x = r.getX(); x < r.getRight(); x += step)
        points.push_back ({ x + jitter(), r.getY() + jitter() });

    for (float y = r.getY(); y < r.getBottom(); y += step)
        points.push_back ({ r.getRight() + jitter(), y + jitter() });

    for (float x = r.getRight(); x > r.getX(); x -= step)
        points.push_back ({ x + jitter(), r.getBottom() + jitter() });

    for (float y = r.getBottom(); y > r.getY(); y -= step)
        points.push_back ({ r.getX() + jitter(), y + jitter() });

    juce::Path path;

    if (points.empty())
        return path;

    path.startNewSubPath (points.front());

    for (size_t i = 1; i < points.size(); ++i)
        path.lineTo (points[i]);

    path.closeSubPath();
    return path;
}

juce::Path HandDrawnLookAndFeel::wobblyEllipse (juce::Rectangle<float> r, int seed, float wobble)
{
    juce::Random random (seed);
    constexpr int numPoints = 40;

    const auto centre = r.getCentre();
    const float rx = r.getWidth() * 0.5f;
    const float ry = r.getHeight() * 0.5f;

    juce::Path path;

    for (int i = 0; i < numPoints; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi * (float) i / (float) numPoints;
        const float wob = (random.nextFloat() * 2.0f - 1.0f) * wobble;
        const juce::Point<float> p (centre.x + (rx + wob) * std::cos (angle),
                                    centre.y + (ry + wob) * std::sin (angle));

        if (i == 0)
            path.startNewSubPath (p);
        else
            path.lineTo (p);
    }

    path.closeSubPath();
    return path;
}

juce::Path HandDrawnLookAndFeel::wobblyLine (juce::Point<float> start, juce::Point<float> end, int seed, float wobble)
{
    juce::Random random (seed);
    const int segments = juce::jmax (2, (int) (start.getDistanceFrom (end) / 10.0f));

    juce::Path path;
    path.startNewSubPath (start);

    for (int i = 1; i <= segments; ++i)
    {
        const float t = (float) i / (float) segments;
        auto p = start + (end - start) * t;

        if (i < segments)
            p += { (random.nextFloat() * 2.0f - 1.0f) * wobble, (random.nextFloat() * 2.0f - 1.0f) * wobble };

        path.lineTo (p);
    }

    return path;
}

// ---------------------------------------------------------------------------
// Fonts
// ---------------------------------------------------------------------------

juce::Font HandDrawnLookAndFeel::getLabelFont (juce::Label&) { return handFont (12.0f, true); }
juce::Font HandDrawnLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return handFont (juce::jmin (13.0f, (float) buttonHeight * 0.6f), true);
}
juce::Font HandDrawnLookAndFeel::getComboBoxFont (juce::ComboBox&) { return handFont (12.0f); }
juce::Font HandDrawnLookAndFeel::getPopupMenuFont() { return handFont (14.0f); }
juce::Font HandDrawnLookAndFeel::getSliderPopupFont (juce::Slider&) { return handFont (13.0f); }

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

void HandDrawnLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                             juce::Slider& slider)
{
    const auto seed = seedFor (slider);
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const float size = juce::jmin (bounds.getWidth(), bounds.getHeight());
    const auto knob = bounds.withSizeKeepingCentre (size, size);
    const auto centre = knob.getCentre();
    const float radius = size * 0.5f;
    const float alpha = slider.isEnabled() ? 1.0f : 0.4f;

    g.setColour (paper());
    g.fillEllipse (knob);

    g.setColour (ink().withAlpha (alpha));
    g.strokePath (wobblyEllipse (knob, seed), juce::PathStrokeType (1.8f));

    // Min/max tick marks
    for (float a : { rotaryStartAngle, rotaryEndAngle })
    {
        const auto outer = centre.getPointOnCircumference (radius + 3.0f, a);
        const auto inner = centre.getPointOnCircumference (radius - 1.0f, a);
        g.drawLine ({ inner, outer }, 1.2f);
    }

    // Pointer
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto tip = centre.getPointOnCircumference (radius * 0.82f, angle);
    const auto tail = centre.getPointOnCircumference (radius * 0.2f, angle);
    g.strokePath (wobblyLine (tail, tip, seed + 1, 0.6f), juce::PathStrokeType (2.2f));
    g.fillEllipse (juce::Rectangle<float> (4.0f, 4.0f).withCentre (centre));
}

void HandDrawnLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    const auto seed = seedFor (button);
    const bool on = button.getToggleState();
    const float alpha = button.isEnabled() ? 1.0f : 0.4f;
    auto bounds = button.getLocalBounds().toFloat();

    if (button.getComponentID() == kFootswitchId)
    {
        const float d = juce::jmin (bounds.getWidth(), bounds.getHeight()) - 6.0f;
        const auto outer = bounds.withSizeKeepingCentre (d, d);
        const auto inner = outer.reduced (d * 0.28f);

        g.setColour (shouldDrawButtonAsDown ? pencil().withAlpha (0.3f) : paper());
        g.fillEllipse (outer);

        g.setColour (ink().withAlpha (alpha));
        g.strokePath (wobblyEllipse (outer, seed, 1.4f), juce::PathStrokeType (2.2f));

        if (on)
            g.fillEllipse (inner);
        else
            g.strokePath (wobblyEllipse (inner, seed + 1), juce::PathStrokeType (1.2f));

        if (shouldDrawButtonAsHighlighted)
            g.strokePath (wobblyEllipse (outer.expanded (3.0f), seed + 2), juce::PathStrokeType (0.8f));

        return;
    }

    // Plain check box: a wobbly square with an X scribbled in it when on.
    const bool hasText = button.getButtonText().isNotEmpty();
    const float boxSize = 16.0f;
    auto box = hasText ? bounds.removeFromLeft (bounds.getHeight()).withSizeKeepingCentre (boxSize, boxSize)
                       : bounds.withSizeKeepingCentre (boxSize, boxSize);

    g.setColour (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted ? pencil().withAlpha (0.2f) : paper());
    g.fillRect (box);

    g.setColour (ink().withAlpha (alpha));
    g.strokePath (wobblyRect (box, seed), juce::PathStrokeType (1.5f));

    if (on)
    {
        const auto inner = box.reduced (3.0f);
        g.strokePath (wobblyLine (inner.getTopLeft(), inner.getBottomRight(), seed + 1, 0.8f), juce::PathStrokeType (1.8f));
        g.strokePath (wobblyLine (inner.getTopRight(), inner.getBottomLeft(), seed + 2, 0.8f), juce::PathStrokeType (1.8f));
    }

    if (hasText)
    {
        g.setFont (handFont (13.0f));
        g.drawFittedText (button.getButtonText(), bounds.reduced (4.0f, 0.0f).toNearestInt(),
                          juce::Justification::centredLeft, 1);
    }
}

void HandDrawnLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                                 bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    const auto seed = seedFor (button);
    const auto bounds = button.getLocalBounds().toFloat().reduced (1.5f);
    const float alpha = button.isEnabled() ? 1.0f : 0.35f;

    auto fill = paper();

    if (button.getToggleState())
        fill = findColour (juce::TextButton::buttonOnColourId);

    if (shouldDrawButtonAsDown)
        fill = pencil().withAlpha (0.35f);
    else if (shouldDrawButtonAsHighlighted)
        fill = pencil().withAlpha (0.15f);

    g.setColour (fill);
    g.fillRect (bounds);

    g.setColour (ink().withAlpha (alpha));
    g.strokePath (wobblyRect (bounds, seed), juce::PathStrokeType (1.5f));
}

void HandDrawnLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                         int, int, int, int, juce::ComboBox& box)
{
    const auto seed = seedFor (box);
    const auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (1.5f);
    const float alpha = box.isEnabled() ? 1.0f : 0.35f;

    g.setColour (paper());
    g.fillRect (bounds);

    g.setColour (ink().withAlpha (alpha));
    g.strokePath (wobblyRect (bounds, seed), juce::PathStrokeType (1.5f));

    // Little hand-drawn "v"
    const float arrowW = 8.0f;
    const auto right = bounds.getRight() - 6.0f;
    const auto midY = bounds.getCentreY();
    g.strokePath (wobblyLine ({ right - arrowW, midY - 2.0f }, { right - arrowW * 0.5f, midY + 3.0f }, seed + 1, 0.5f),
                  juce::PathStrokeType (1.5f));
    g.strokePath (wobblyLine ({ right - arrowW * 0.5f, midY + 3.0f }, { right, midY - 2.0f }, seed + 2, 0.5f),
                  juce::PathStrokeType (1.5f));
}

void HandDrawnLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (paper());
    g.setColour (ink());
    g.strokePath (wobblyRect (juce::Rectangle<float> (1.0f, 1.0f, (float) width - 2.0f, (float) height - 2.0f), 7),
                  juce::PathStrokeType (1.5f));
}

void HandDrawnLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    const auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);

    g.fillAll (paper());
    g.setColour (ink());
    g.strokePath (wobblyRect (bounds.reduced (1.5f), 11), juce::PathStrokeType (1.5f));

    juce::AttributedString attributed;
    attributed.setJustification (juce::Justification::centredLeft);
    attributed.append (text, handFont (14.0f), ink());

    juce::TextLayout layout;
    layout.createLayoutWithBalancedLineLengths (attributed, (float) width - 14.0f);
    layout.draw (g, bounds.reduced (7.0f, 4.0f));
}

} // namespace fpb
