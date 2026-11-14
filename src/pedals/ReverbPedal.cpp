#include "ReverbPedal.h"

#include "ui/ParameterPanel.h"

namespace fpb
{

juce::String ReverbPedal::getAlgorithmCredit() const
{
    return "Freeverb: Schroeder/Moorer reverb (8 comb + 4 all-pass filters per channel) "
           "by Jezar at Dreampoint, public domain. Implementation: juce::dsp::Reverb "
           "(JUCE framework, AGPLv3).";
}

void ReverbPedal::addParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    using namespace juce;

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { paramId (pSize), 1 }, "Reverb Size",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { paramId (pDamping), 1 }, "Reverb Damping",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { paramId (pWidth), 1 }, "Reverb Width",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { paramId (pMix), 1 }, "Reverb Mix",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.3f));
}

void ReverbPedal::attachParameters (juce::AudioProcessorValueTreeState& apvts)
{
    size_ = apvts.getRawParameterValue (paramId (pSize));
    damping_ = apvts.getRawParameterValue (paramId (pDamping));
    width_ = apvts.getRawParameterValue (paramId (pWidth));
    mix_ = apvts.getRawParameterValue (paramId (pMix));

    jassert (size_ != nullptr && damping_ != nullptr && width_ != nullptr && mix_ != nullptr);
}

void ReverbPedal::prepare (const juce::dsp::ProcessSpec& spec)
{
    reverb_.prepare (spec);
    reset();
}

void ReverbPedal::reset()
{
    reverb_.reset();
}

void ReverbPedal::process (juce::dsp::AudioBlock<float>& block)
{
    const float mix = mix_->load();

    juce::dsp::Reverb::Parameters params;
    params.roomSize = size_->load();
    params.damping = damping_->load();
    params.width = width_->load();
    params.wetLevel = mix;
    params.dryLevel = 1.0f - mix;
    params.freezeMode = 0.0f;
    reverb_.setParameters (params);

    juce::dsp::ProcessContextReplacing<float> context (block);
    reverb_.process (context);
}

std::unique_ptr<juce::Component> ReverbPedal::createEditor (juce::AudioProcessorValueTreeState& apvts)
{
    return std::make_unique<ParameterPanel> (apvts, std::vector<ParameterPanel::Item> {
        { paramId (pSize), "SIZE" },
        { paramId (pDamping), "DAMP" },
        { paramId (pWidth), "WIDTH" },
        { paramId (pMix), "MIX" },
    });
}

} // namespace fpb
