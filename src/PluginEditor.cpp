#include "PluginEditor.h"

namespace fpb
{

FossPedalboardEditor::FossPedalboardEditor (FossPedalboardProcessor& processor)
    : juce::AudioProcessorEditor (processor),
      processor_ (processor),
      board_ (processor.getChain(), processor.getValueTreeState())
{
    setLookAndFeel (&lookAndFeel_);
    addAndMakeVisible (board_);

    board_.onScaleChanged = [this] (float scale)
    {
        processor_.setUiScale (scale);
        applyScale (scale);
    };

    const float scale = processor_.getUiScale();
    board_.setScaleSelection (scale);
    setResizable (false, false);
    applyScale (scale);
}

FossPedalboardEditor::~FossPedalboardEditor()
{
    setLookAndFeel (nullptr);
}

void FossPedalboardEditor::applyScale (float scale)
{
    setSize (BoardComponent::kWidth, BoardComponent::kHeight);
    setScaleFactor (scale);
}

void FossPedalboardEditor::paint (juce::Graphics& g)
{
    g.fillAll (HandDrawnLookAndFeel::paper());
}

void FossPedalboardEditor::resized()
{
    board_.setBounds (getLocalBounds());
}

} // namespace fpb
