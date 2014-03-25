#include "Backtrack.h"

#include <algorithm> /* std::reverse() */
#include <cassert>
#include <fhg/assert.hpp>

#include "State.h"

namespace jpn {
namespace analysis {

std::vector<TransitionId> backtrack(const State *state, const State *lastState) {
    assert(state != nullptr);
    assert(lastState != nullptr);

    std::vector<TransitionId> result;

    while (state != lastState) {
        result.push_back(state->lastTransition()->id());
        state = state->previous();
    }

    std::reverse(result.begin(), result.end());

    return result;
}

}} // namespace jpn::analysis

/* vim:set et sts=4 sw=4: */
