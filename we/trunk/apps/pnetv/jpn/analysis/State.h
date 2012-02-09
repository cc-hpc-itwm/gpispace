#pragma once

#include <jpn/Transition.h>
#include <jpn/Marking.h>

namespace jpn {
namespace analysis {

/**
 * State of a Petri net.
 */
class State {
    Marking marking_; ///< Marking.
    const Transition *lastTransition_; ///< Last transition, firing of which has moved us to the state.
    const State *previous_; ///< Previous state, from which we have been moved to the state.

    public:

    /**
     * Constructor.
     *
     * \param[in] marking Marking.
     * \param[in] lastTransition Last transition, firing of which has moved us to the state.
     * \param[in] previous Previous state, from which we have been moved to the state.
     */
    State(const Marking &marking, const Transition *lastTransition, const State *previous):
        marking_(marking), lastTransition_(lastTransition), previous_(previous)
    {}

    /**
     * \return Marking.
     */
    const Marking &marking() const { return marking_; }

    /**
     * \return Last transition, firing of which has moved us to the state.
     */
    const Transition *lastTransition() const { return lastTransition_; }

    /**
     * \return Previous state, from which we have been moved to the state.
     */
    const State *previous() const { return previous_; }
};

}} // namespace jpn::analysis

/* vim:set et sts=4 sw=4: */
