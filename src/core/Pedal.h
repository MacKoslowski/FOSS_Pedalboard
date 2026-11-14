#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include <memory>

namespace fpb
{

/** Host transport information handed to pedals once per block. */
struct TransportInfo
{
    double bpm = 120.0;
    bool playing = false;
};

/** Abstract base class for every effect on the board.

    One instance of each concrete pedal type is created at plugin construction
    and lives for the whole session. A board slot merely points at one of them.

    Lifecycle:
      1. addParameters()    - called once while the parameter layout is built
      2. attachParameters() - called once after the APVTS exists
      3. prepare()/reset()  - called from prepareToPlay / when assigned to a slot
      4. process()          - audio thread, realtime-safe, in place
      5. createEditor()     - message thread, builds the knob panel for the UI

    Realtime rules for process(): no allocation, locks, logging, exceptions or
    message-thread calls. Read parameters through the atomics cached in
    attachParameters().
*/
class Pedal
{
public:
    virtual ~Pedal() = default;

    /** Stable lowercase id used in parameter ids and saved state, e.g. "delay". */
    [[nodiscard]] virtual juce::String getTypeId() const = 0;

    /** Handwritten label shown on the pedal, e.g. "DELAY". */
    [[nodiscard]] virtual juce::String getDisplayName() const = 0;

    /** Text for the "(i)" tooltip: algorithm, author/source and license. */
    [[nodiscard]] virtual juce::String getAlgorithmCredit() const = 0;

    /** Appends this pedal's parameters to the plugin's parameter layout. */
    virtual void addParameters (juce::AudioProcessorValueTreeState::ParameterLayout& layout) = 0;

    /** Caches raw atomic parameter pointers for use on the audio thread. */
    virtual void attachParameters (juce::AudioProcessorValueTreeState& apvts) = 0;

    virtual void prepare (const juce::dsp::ProcessSpec& spec) = 0;
    virtual void reset() = 0;

    /** Optional: receive host tempo/transport. Default ignores it. */
    virtual void setTransport (const TransportInfo&) {}

    /** Processes audio in place. Channel count is 1 or 2. */
    virtual void process (juce::dsp::AudioBlock<float>& block) = 0;

    /** Builds the control panel shown inside the pedal's enclosure. */
    [[nodiscard]] virtual std::unique_ptr<juce::Component> createEditor (juce::AudioProcessorValueTreeState& apvts) = 0;

    /** Builds a fully-qualified parameter id for this pedal. */
    [[nodiscard]] juce::String paramId (const char* paramName) const
    {
        return getTypeId() + "_" + paramName;
    }
};

} // namespace fpb
