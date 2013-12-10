#include "StateSpace.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include "Backtrack.h"

namespace jpn {
namespace analysis {

bool karpMiller(const std::vector<Transition> &transitions, const Marking &initialMarking, boost::ptr_vector<State> &states) {
    /* Initialize the queue of states. */
    states.clear();
    states.push_back(new State(initialMarking, NULL, NULL));

    std::vector<const Transition *> enabledTransitions;

    for (std::size_t i = 0; i < states.size(); ++i) {
        const State *currentState = &states[i];

        enabledTransitions.clear();
        FOREACH (const Transition &transition, transitions) {
            if (transition.canFire(currentState->marking())) {
                if (!enabledTransitions.empty()) {
                    if (enabledTransitions.front()->priority() < transition.priority()) {
                        enabledTransitions.clear();
                    } else if (enabledTransitions.front()->priority() > transition.priority()) {
                        continue;
                    }
                }
                enabledTransitions.push_back(&transition);
            }
        }

        FOREACH (const Transition *transition, enabledTransitions) {
            /* Simple partial order reduction taking only O(1) time. */
            if (currentState->previous() && transition->id() < currentState->lastTransition()->id() &&
                transition->canFire(currentState->previous()->marking()) &&
                currentState->lastTransition()->canFire(transition->fire(currentState->previous()->marking())) &&
                currentState->lastTransition()->priority() == transition->priority()) {
                continue;
            }

            /* Fire the transition. */
            Marking newMarking = transition->fire(currentState->marking());

            /* Accelerate. */
            for (const State *state = currentState; state != NULL; state = state->previous()) {
                if (state->marking() < newMarking) {
                    #ifdef JPN_EXTENDED_MARKINGS
                        newMarking = accelerate(state->marking(), newMarking);
                    #else
                        return false;
                    #endif
                }
            }

            /* State space reduction. */
            bool covered = false;
            FOREACH (const State &state, states) {
                if (newMarking <= state.marking()) {
                    covered = true;
                    break;
                }
            }
            if (covered) {
                continue;
            }

            /* Enqueue the new state and mark it as visited. */
            states.push_back(new State(newMarking, transition, currentState));
        }
    }

    return true;
}

bool findLoop(const std::vector<Transition> &transitions, const Marking &initialMarking, std::vector<TransitionId> &init, std::vector<TransitionId> &loop) {
    /* Initialize the queue of states. */
    boost::ptr_vector<State> states;
    states.push_back(new State(initialMarking, NULL, NULL));

    std::vector<const Transition *> enabledTransitions;

    for (std::size_t i = 0; i < states.size(); ++i) {
        const State *currentState = &states[i];

        enabledTransitions.clear();
        FOREACH (const Transition &transition, transitions) {
            if (transition.canFire(currentState->marking())) {
                if (!enabledTransitions.empty()) {
                    if (enabledTransitions.front()->priority() < transition.priority()) {
                        enabledTransitions.clear();
                    } else if (enabledTransitions.front()->priority() > transition.priority()) {
                        continue;
                    }
                }
                enabledTransitions.push_back(&transition);
            }
        }

        FOREACH (const Transition *transition, enabledTransitions) {
            /* Simple partial order reduction taking only O(1) time. */
            if (currentState->previous() && transition->id() < currentState->lastTransition()->id() &&
                transition->canFire(currentState->previous()->marking()) &&
                currentState->lastTransition()->canFire(transition->fire(currentState->previous()->marking())) &&
                currentState->lastTransition()->priority() == transition->priority()) {
                continue;
            }

            /* Fire the transition. */
            Marking newMarking = transition->fire(currentState->marking());

            /* Accelerate. */
            for (const State *state = currentState; state != NULL; state = state->previous()) {
                if (state->marking() <= newMarking) {
                    /* We are done. */
                    init = backtrack(state, &states[0]);
                    loop = backtrack(currentState, state);
                    loop.push_back(transition->id());

                    return true;
                }
            }

            /* State space reduction. */
            bool covered = false;
            FOREACH (const State &state, states) {
                if (newMarking <= state.marking()) {
                    covered = true;
                    break;
                }
            }
            if (covered) {
                continue;
            }

            /* Enqueue the new state and mark it as visited. */
            states.push_back(new State(newMarking, transition, currentState));
        }
    }

    return false;
}
}} // namespace jpn::analysis

/* vim:set et sts=4 sw=4: */
