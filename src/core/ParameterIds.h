#pragma once

#include <juce_core/juce_core.h>

namespace fpb
{

/** Identifiers for host-visible parameters and for the saved board state.

    Pedal parameters follow the pattern "<pedalTypeId>_<paramName>", built with
    Pedal::paramId(). Only the global (non-pedal) ids live here.
*/
namespace ParamIds
{
    constexpr const char* inputGain  = "in_gain";
    constexpr const char* outputGain = "out_gain";
} // namespace ParamIds

/** Property and node names used in the ValueTree that stores the board. */
namespace StateIds
{
    constexpr const char* board   = "BOARD";
    constexpr const char* slot    = "SLOT";
    constexpr const char* pedal   = "pedal";
    constexpr const char* bypass  = "bypass";
    constexpr const char* uiScale = "uiScale";
} // namespace StateIds

} // namespace fpb
