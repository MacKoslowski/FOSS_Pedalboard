#pragma once

#include "PluginProcessor.h"
#include "ui/BoardComponent.h"
#include "ui/HandDrawnLookAndFeel.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace fpb
{

/** Top-level editor: installs the hand-drawn look, hosts the board and
    applies the user's UI scale. */
class FossPedalboardEditor : public juce::AudioProcessorEditor
{
public:
    explicit FossPedalboardEditor (FossPedalboardProcessor& processor);
    ~FossPedalboardEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void applyScale (float scale);

    FossPedalboardProcessor& processor_;
    HandDrawnLookAndFeel lookAndFeel_;
    BoardComponent board_;
    juce::TooltipWindow tooltipWindow_ { this, 400 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FossPedalboardEditor)
};

} // namespace fpb
