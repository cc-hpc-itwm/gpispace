#include "Termination.h"

#include <functional>

#include "FindAscendingPath.h"

namespace jpn { namespace analysis {

std::vector<TransitionId> findLivelock(const std::vector<Transition> &transitions, const Marking &initialMarking) {
    return findAscendingPath(transitions, initialMarking, std::equal_to<Marking>());
}

std::vector<TransitionId> findUnboundedness(const std::vector<Transition> &transitions, const Marking &initialMarking) {
    return findAscendingPath(transitions, initialMarking, std::less<Marking>());
}

std::vector<TransitionId> findEndlessLoop(const std::vector<Transition> &transitions, const Marking &initialMarking) {
    return findAscendingPath(transitions, initialMarking, std::less_equal<Marking>());
}

}} // namespace jpn::analysis

/* vim:set et sts=4 sw=4: */
