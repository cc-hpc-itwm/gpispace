#include "Verification.h"

#include <jpn/common/Foreach.h>
#include <jpn/common/PrintRange.h>
#include <jpn/analysis/Termination.h>

#include "PetriNet.h"

namespace pnetv {

void VerificationResult::print(std::ostream &out) const {
    out << "(";
    switch (result()) {
        case TERMINATES:
            out << "TERMINATES";
            break;
        case UNBOUNDED:
            out << "UNBOUNDED";
            break;
        case MAYBE_UNBOUNDED:
            out << "MAYBE_UNBOUNDED";
            break;
        case INFINITE:
            out << "INFINITE";
            break;
        case MAYBE_INFINITE:
            out << "MAYBE_INFINITE";
            break;
        default:
            assert(!"NEVER REACHED");
    }
    if (result() != TERMINATES) {
        jpn::printRange(out, trace());
    }
    out << ")";
}

namespace {

inline jpn::Marking makeInitialMarking(const std::vector<Place> &places) {
    std::vector<jpn::PlaceMarking> placeMarkings;
    PlaceId id = 0;
    foreach (const Place &place, places) {
        if (place.initialMarking()) {
            placeMarkings.push_back(jpn::PlaceMarking(id, place.initialMarking()));
        }
        ++id;
    }
    return jpn::Marking(placeMarkings);
}

inline jpn::Marking makeMarking(const std::vector<PlaceId> &placeIds) {
    std::vector<jpn::PlaceMarking> placeMarkings;
    foreach (PlaceId place, placeIds) {
        placeMarkings.push_back(jpn::PlaceMarking(place, 1));
    }
    return jpn::Marking(placeMarkings);
}

jpn::Transition makeTransition(jpn::TransitionId id, const Transition &transition) {
    return jpn::Transition(id, makeMarking(transition.inputPlaces()), makeMarking(transition.outputPlaces()));
}

} // anonymous namespace

VerificationResult verify(const PetriNet &petriNet) {
    jpn::Marking initialMarking = makeInitialMarking(petriNet.places());

    std::vector<jpn::Transition> transitions;

    TransitionId id = 0;
    foreach(const Transition &transition, petriNet.transitions()) {
        if (transition.conditionIsAlwaysTrue()) {
            transitions.push_back(makeTransition(id, transition));
        }
        ++id;
    }

    std::vector<TransitionId> trace;

    trace = jpn::analysis::findUnboundedness(transitions, initialMarking);
    if (!trace.empty()) {
        return VerificationResult(VerificationResult::UNBOUNDED, trace);
    }

    trace = jpn::analysis::findLivelock(transitions, initialMarking);
    if (!trace.empty()) {
        return VerificationResult(VerificationResult::INFINITE, trace);
    }

    id = 0;
    foreach(const Transition &transition, petriNet.transitions()) {
        if (!transition.conditionIsAlwaysTrue()) {
            transitions.push_back(makeTransition(id, transition));
        }
        ++id;
    }

    trace = jpn::analysis::findUnboundedness(transitions, initialMarking);
    if (!trace.empty()) {
        return VerificationResult(VerificationResult::MAYBE_UNBOUNDED, trace);
    }

    trace = jpn::analysis::findLivelock(transitions, initialMarking);
    if (!trace.empty()) {
        return VerificationResult(VerificationResult::MAYBE_INFINITE, trace);
    }

    return VerificationResult(VerificationResult::TERMINATES);
}

} // namespace pnetv

/* vim:set et sts=4 sw=4: */
