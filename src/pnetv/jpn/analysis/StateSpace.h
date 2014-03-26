#pragma once

#include "State.h"

#include <pnetv/jpn/Marking.h>
#include <pnetv/jpn/Transition.h>

#include <boost/ptr_container/ptr_vector.hpp>

#include <vector>

namespace jpn {
namespace analysis {

/**
 * For a Petri net, builds a set of reachable markings covering the set of all reachable markings.
 *
 * \param[in] transitions Transitions of a Petri net.
 * \param[in] initialMarking Initial marking of the Petri net.
 * \param[out] states Computed set of states.
 *
 * \return True on success, false if the Petri net is unbounded and support for extended markings is off.
 */
bool karpMiller(const std::vector<Transition> &transitions, const Marking &initialMarking, boost::ptr_vector<State> &states);

/**
 * For a Petri net, tries to find a loop.
 *
 * \param[in] transitions Transitions of a Petri net.
 * \param[in] initialMarking Initial marking of the Petri net.
 * \param[out] init Ids of transitions preceding the loop.
 * \param[out] loop Ids of transitions constituting the loop.
 *
 * \return True of the loop was found. False otherwise.
 */
bool findLoop(const std::vector<Transition> &transitions, const Marking &initialMarking, std::vector<we::transition_id_type> &init, std::vector<we::transition_id_type> &loop);

}} // namespace jpn::analysis
