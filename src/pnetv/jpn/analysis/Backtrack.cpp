#include "Backtrack.h"

#include "State.h"

#include <fhg/assert.hpp>

#include <algorithm> /* std::reverse() */
#include <cassert>

namespace jpn {
namespace analysis {

std::vector<we::transition_id_type> backtrack(const State *state, const State *lastState) {
    assert(state != nullptr);
    assert(lastState != nullptr);

    std::vector<we::transition_id_type> result;

    while (state != lastState) {
        result.push_back(state->lastTransition()->id());
        state = state->previous();
    }

    std::reverse(result.begin(), result.end());

    return result;
}

}} // namespace jpn::analysis
