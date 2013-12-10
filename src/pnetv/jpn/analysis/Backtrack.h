#pragma once

#include <vector>

#include <jpn/Types.h>

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
std::vector<TransitionId> backtrack(const State *state, const State *lastState);

}} // namespace jpn::analysis

/* vim:set et sts=4 sw=4: */
