#include "Verification.h"

#include <jpn/common/Foreach.h>
#include <jpn/common/PrintRange.h>
#include <jpn/analysis/StateSpace.h>

#include "PetriNet.h"

namespace jpna {

namespace {

inline jpn::Marking makeInitialMarking(const std::vector<const Place *> &places) {
    std::vector<jpn::PlaceMarking> placeMarkings;
    FOREACH (const Place *place, places) {
        if (place->initialMarking()) {
            placeMarkings.push_back(jpn::PlaceMarking(place->id(), place->initialMarking()));
        }
    }
    return jpn::Marking(placeMarkings);
}

inline jpn::Marking makeMarking(const std::vector<const Place *> &places) {
    std::vector<jpn::PlaceMarking> placeMarkings;
    FOREACH (const Place *place, places) {
        placeMarkings.push_back(jpn::PlaceMarking(place->id(), 1));
    }
    return jpn::Marking(placeMarkings);
}

jpn::Transition makeTransition(const Transition *transition) {
    return jpn::Transition(
        transition->id(),
        makeMarking(transition->inputPlaces()),
        makeMarking(transition->outputPlaces()),
        transition->priority());
}

std::vector<const Transition *> makeTrace(const std::vector<jpn::TransitionId> &trace, const PetriNet &petriNet) {
    std::vector<const Transition *> result;
    FOREACH (jpn::TransitionId transitionId, trace) {
        result.push_back(petriNet.getTransition(transitionId));
    }
    return result;
}

} // anonymous namespace

VerificationResult verify(const PetriNet &petriNet) {
    jpn::Marking initialMarking = makeInitialMarking(petriNet.places());

    std::vector<jpn::Transition> transitions;

    FOREACH(const Transition *transition, petriNet.transitions()) {
        if (transition->conditionAlwaysTrue()) {
            transitions.push_back(makeTransition(transition));
        }
    }

    std::vector<TransitionId> init;
    std::vector<TransitionId> loop;
    if (jpn::analysis::findLoop(transitions, initialMarking, init, loop)) {
        return VerificationResult(VerificationResult::LOOPS, makeTrace(init, petriNet), makeTrace(loop, petriNet));
    }

    FOREACH(const Transition *transition, petriNet.transitions()) {
        if (!transition->conditionAlwaysTrue()) {
            transitions.push_back(makeTransition(transition));
        }
    }

    if (jpn::analysis::findLoop(transitions, initialMarking, init, loop)) {
        return VerificationResult(VerificationResult::MAYBE_LOOPS, makeTrace(init, petriNet), makeTrace(loop, petriNet));
    }

    return VerificationResult(VerificationResult::TERMINATES);
}

} // namespace jpna

/* vim:set et sts=4 sw=4: */
