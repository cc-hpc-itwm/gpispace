#pragma once

#include <vector>

#include <we/type/id.hpp>

namespace jpn {
namespace analysis {

class State;

/**
 * Enumerates all transitions leading from lastState to state.
 *
 * \param state First state to visit during backtracking.
 * \param lastState Last state to visit during backtracking.
 *
 * \return Ids of transitions leading from `lastState' to `state'.
 */
std::vector<we::transition_id_type> backtrack(const State *state, const State *lastState);

}} // namespace jpn::analysis
