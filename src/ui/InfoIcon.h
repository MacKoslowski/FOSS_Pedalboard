#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace fpb
{

/** The little hand-drawn "(i)" on every pedal. Hovering it shows the pedal's
    algorithm credit in a tooltip; the same text is exposed to screen readers. */
class InfoIcon : public juce::Component,
                 public juce::SettableTooltipClient
{
public:
    explicit InfoIcon (const juce::String& credit);

    void paint (juce::Graphics& g) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    bool hovered_ = false;
};

} // namespace fpb
