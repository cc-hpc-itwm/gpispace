#include "StateSpace.h"

#include "Backtrack.h"

#include <boost/ptr_container/ptr_vector.hpp>

namespace jpn {
namespace analysis {

bool findLoop(const std::vector<Transition> &transitions, const Marking &initialMarking, std::vector<we::transition_id_type> &init, std::vector<we::transition_id_type> &loop) {
    /* Initialize the queue of states. */
    boost::ptr_vector<State> states;
    states.push_back(new State(initialMarking, nullptr, nullptr));

    std::vector<const Transition *> enabledTransitions;

    for (std::size_t i = 0; i < states.size(); ++i) {
        const State *currentState = &states[i];

        enabledTransitions.clear();
        for (const Transition &transition : transitions) {
            if (transition.canFire(currentState->marking())) {
                if (!enabledTransitions.empty()) {
                    if (enabledTransitions.front()->priority() < transition.priority()) {
                        enabledTransitions.clear();
                    } else if (enabledTransitions.front()->priority() > transition.priority()) {
                        continue;
                    }
                }
                enabledTransitions.emplace_back(&transition);
            }
        }

        for (const Transition *transition : enabledTransitions) {
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
            for (const State *state = currentState; state != nullptr; state = state->previous()) {
                if (state->marking() <= newMarking) {
                    /* We are done. */
                    init = backtrack(state, &states[0]);
                    loop = backtrack(currentState, state);
                    loop.emplace_back(transition->id());

                    return true;
                }
            }

            /* State space reduction. */
            bool covered = false;
            for (const State &state : states) {
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
