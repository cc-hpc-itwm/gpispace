#include "Verification.h"

#include <jpn/common/Foreach.h>
#include <jpn/common/PrintRange.h>
#include <jpn/analysis/Termination.h>

#include "PetriNet.h"

namespace pneta {

namespace {

inline jpn::Marking makeInitialMarking(const std::vector<const Place *> &places) {
    std::vector<jpn::PlaceMarking> placeMarkings;
    foreach (const Place *place, places) {
        if (place->initialMarking()) {
            placeMarkings.push_back(jpn::PlaceMarking(place->id(), place->initialMarking()));
        }
    }
    return jpn::Marking(placeMarkings);
}

inline jpn::Marking makeMarking(const std::vector<const Place *> &places) {
    std::vector<jpn::PlaceMarking> placeMarkings;
    foreach (const Place *place, places) {
        placeMarkings.push_back(jpn::PlaceMarking(place->id(), 1));
    }
    return jpn::Marking(placeMarkings);
}

jpn::Transition makeTransition(const Transition *transition) {
    return jpn::Transition(
        transition->id(),
        makeMarking(transition->inputPlaces()),
        makeMarking(transition->outputPlaces()),
        transition->firesFinitely());
}

std::vector<const Transition *> makeTrace(const std::vector<jpn::TransitionId> &trace, const PetriNet &petriNet) {
    std::vector<const Transition *> result;
    foreach (jpn::TransitionId transitionId, trace) {
        result.push_back(petriNet.getTransition(transitionId));
    }
    return result;
}

} // anonymous namespace

VerificationResult verify(const PetriNet &petriNet) {
    jpn::Marking initialMarking = makeInitialMarking(petriNet.places());

    std::vector<jpn::Transition> transitions;

    foreach(const Transition *transition, petriNet.transitions()) {
        if (transition->conditionAlwaysTrue()) {
            transitions.push_back(makeTransition(transition));
        }
    }

    std::vector<TransitionId> trace;

    trace = jpn::analysis::findUnboundedness(transitions, initialMarking);
    if (!trace.empty()) {
        return VerificationResult(VerificationResult::UNBOUNDED, makeTrace(trace, petriNet));
    }

    trace = jpn::analysis::findLivelock(transitions, initialMarking);
    if (!trace.empty()) {
        return VerificationResult(VerificationResult::INFINITE, makeTrace(trace, petriNet));
    }

    foreach(const Transition *transition, petriNet.transitions()) {
        if (!transition->conditionAlwaysTrue()) {
            transitions.push_back(makeTransition(transition));
        }
    }

    trace = jpn::analysis::findUnboundedness(transitions, initialMarking);
    if (!trace.empty()) {
        return VerificationResult(VerificationResult::MAYBE_UNBOUNDED, makeTrace(trace, petriNet));
    }

    trace = jpn::analysis::findLivelock(transitions, initialMarking);
    if (!trace.empty()) {
        return VerificationResult(VerificationResult::MAYBE_INFINITE, makeTrace(trace, petriNet));
    }

    return VerificationResult(VerificationResult::TERMINATES);
}

} // namespace pneta

/* vim:set et sts=4 sw=4: */
