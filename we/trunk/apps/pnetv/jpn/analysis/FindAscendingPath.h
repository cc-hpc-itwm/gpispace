#pragma once

#include <jpn/config.h>

#include <algorithm> /* reverse */

#include <boost/ptr_container/ptr_vector.hpp> 

#include <jpn/common/Foreach.h>

#include "State.h"

namespace jpn {
namespace analysis {

struct Deleter {
    template<class T>
    void operator()(const T *x) { delete x; }
};

/**
 * Finds a path in a Petri net visiting first a marking m1 and then a marking m2 such than compare(m1, m2) is true.
 *
 * \param transitions Transitions of a Petri net.
 * \param initialMarking Initial marking.
 * \param compare Binary function returning true iff the marking given as its left argument is bigger then the marking given as its right argument.
 * \tparam Compare Type of the comparator.
 *
 * \return Vector of transitions ids representing the path. If no path is found, empty vector is returned.
 */
template<class Compare>
std::vector<TransitionId>
findAscendingPath(
    const std::vector<Transition> &transitions,
    const Marking &initialMarking,
    Compare compare)
{
    /* States queue. */
    boost::ptr_vector<State> states;

    states.push_back(new State(initialMarking, NULL, NULL));

    for (std::size_t i = 0; i < states.size(); ++i) {
        const State *currentState = &states[i];

        foreach (const Transition &transition, transitions) {
            if (transition.canFire(currentState->marking())) {
                /* Simple partial order reduction. */
                if (currentState->previous() && transition.id() < currentState->lastTransition()->id() &&
                    transition.canFire(currentState->previous()->marking()) &&
                    currentState->lastTransition()->canFire(transition.fire(currentState->previous()->marking()))) {
                    continue;
                }

                /* Fire the transition. */
                Marking newMarking = transition.fire(currentState->marking());

                /* Do we exceed some of the predecessor markings? */
                for (const State *state = currentState; state != NULL; state = state->previous()) {
                    if (compare(state->marking(), newMarking)) {
                        /* Restore the path to the new marking. */
                        std::vector<TransitionId> result;

                        result.push_back(transition.id());
                        for (const State *state = currentState; state->previous() != NULL; state = state->previous()) {
                            result.push_back(state->lastTransition()->id());
                        }
                        std::reverse(result.begin(), result.end());

                        return result;
                    }
                }

                /* State space reduction. */
                bool covered = false;
                foreach (const State &state, states) {
                    if (newMarking <= state.marking()) {
                        covered = true;
                        break;
                    }
                }
                if (covered) {
                    continue;
                }

                /* Enqueue the new state. */
                states.push_back(new State(newMarking, &transition, currentState));
            }
        }
    }

    return std::vector<TransitionId>();
}

}} // namespace jpn::analysis

/* vim:set et sts=4 sw=4: */
