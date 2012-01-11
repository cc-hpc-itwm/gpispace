#pragma once

#include <jpn/config.h>

#include <vector>

#include <jpn/Transition.h>

namespace jpn {

class Marking;

namespace analysis {

std::vector<TransitionId> findLivelock(const std::vector<Transition> &transitions, const Marking &initialMarking);
std::vector<TransitionId> findUnboundedness(const std::vector<Transition> &transitions, const Marking &initialMarking);
std::vector<TransitionId> findEndlessLoop(const std::vector<Transition> &transitions, const Marking &initialMarking);

}} // namespace jpn::analysis

/* vim:set et sts=4 sw=4: */
