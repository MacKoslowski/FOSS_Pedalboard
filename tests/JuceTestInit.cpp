// Brings up JUCE's message manager for the duration of the test run so that
// AudioProcessorValueTreeState (which uses a Timer) can be constructed.

#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

namespace
{
class JuceTestInit : public Catch::EventListenerBase
{
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting (const Catch::TestRunInfo&) override
    {
        initialiser_ = std::make_unique<juce::ScopedJuceInitialiser_GUI>();
    }

    void testRunEnded (const Catch::TestRunStats&) override
    {
        initialiser_.reset();
    }

private:
    std::unique_ptr<juce::ScopedJuceInitialiser_GUI> initialiser_;
};
} // namespace

CATCH_REGISTER_LISTENER (JuceTestInit)
