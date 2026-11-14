#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace fpb
{

/** The deliberately amateurish look: black ink on cream paper, wobbly outlines,
    handwritten-style uppercase labels.

    Everything here is a procedural placeholder. As the user supplies drawings
    per UI element they replace the procedural paths with artwork from assets/.
    Seeds are derived from the component address so a given control keeps the
    same wobble between repaints instead of jittering.
*/
class HandDrawnLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HandDrawnLookAndFeel();

    static juce::Colour ink() { return juce::Colour (0xff1b1b1b); }
    static juce::Colour paper() { return juce::Colour (0xfffdfbf4); }
    static juce::Colour pencil() { return juce::Colour (0xff9a9a9a); }

    static juce::Font handFont (float height, bool bold = false);

    static juce::Path wobblyRect (juce::Rectangle<float> area, int seed, float wobble = 1.2f);
    static juce::Path wobblyEllipse (juce::Rectangle<float> area, int seed, float wobble = 1.0f);
    static juce::Path wobblyLine (juce::Point<float> start, juce::Point<float> end, int seed, float wobble = 1.0f);
    static int seedFor (const juce::Component& component);

    // Component id that makes a ToggleButton draw as a stomp switch.
    static constexpr const char* kFootswitchId = "footswitch";

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height, float sliderPosProportional,
                           float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown, int buttonX, int buttonY,
                       int buttonW, int buttonH, juce::ComboBox&) override;

    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;

    void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;

    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getSliderPopupFont (juce::Slider&) override;
};

} // namespace fpb
