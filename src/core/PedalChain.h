#pragma once

#include "Pedal.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <vector>

namespace fpb
{

/** The board: a fixed number of slots, each pointing at one pedal instance.

    Mutations (setPedal, swapSlots, setBypassed, ...) happen on the message
    thread. The complete slot layout is packed into a single 64-bit atomic so
    the audio thread always sees a consistent board without locking.

    Bypass toggles are crossfaded over a few milliseconds so stomping a pedal
    never clicks. Assigning a pedal to a slot resets its state on the audio
    thread before it is first heard.

    Listeners (the UI) are told about changes through ChangeBroadcaster.
*/
class PedalChain : public juce::ChangeBroadcaster
{
public:
    static constexpr int kMaxSlots = 6;
    static constexpr int kNoPedal = -1;

    struct SlotState
    {
        int pedalIndex = kNoPedal;   // index into the pedal instance list, or kNoPedal
        bool bypassed = false;

        bool operator== (const SlotState&) const = default;
    };

    using Slots = std::array<SlotState, kMaxSlots>;

    /** The chain does not own the pedals; the processor does. */
    explicit PedalChain (std::vector<std::unique_ptr<Pedal>>& pedals);

    // Audio thread -------------------------------------------------------
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void setTransport (const TransportInfo& transport);
    void process (juce::dsp::AudioBlock<float>& block);

    // Message thread -----------------------------------------------------
    [[nodiscard]] Slots getSlots() const;
    [[nodiscard]] SlotState getSlot (int slot) const;
    void setSlots (const Slots& slots);

    /** Puts a pedal in a slot. Fails (returns false) if that pedal type is
        already on the board in a different slot. kNoPedal clears the slot. */
    bool setPedal (int slot, int pedalIndex);
    void clearSlot (int slot);
    void swapSlots (int slotA, int slotB);
    void setBypassed (int slot, bool shouldBeBypassed);

    /** Returns the slot holding this pedal, or kNoPedal if it is not on the board. */
    [[nodiscard]] int findSlotOf (int pedalIndex) const;

    [[nodiscard]] int getNumPedalTypes() const { return (int) pedals_.size(); }
    [[nodiscard]] Pedal& getPedal (int pedalIndex) const { return *pedals_[(size_t) pedalIndex]; }
    [[nodiscard]] int indexOfTypeId (const juce::String& typeId) const;

    /** Persists the board into/out of a parent ValueTree (a BOARD child node). */
    void writeTo (juce::ValueTree& parent) const;
    void readFrom (const juce::ValueTree& parent);

private:
    using Packed = std::uint64_t;

    static Packed pack (const Slots& slots);
    static Slots unpack (Packed packed);
    void publish (const Slots& slots);

    struct SlotRuntime
    {
        int lastPedalIndex = kNoPedal;
        juce::LinearSmoothedValue<float> mix { 0.0f };
    };

    std::vector<std::unique_ptr<Pedal>>& pedals_;
    std::atomic<Packed> packed_ { 0 };
    std::array<SlotRuntime, kMaxSlots> runtime_;
    juce::AudioBuffer<float> dryBuffer_;

    static constexpr double kBypassRampSeconds = 0.02;
};

} // namespace fpb
