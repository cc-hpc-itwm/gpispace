#include "Verification.h"

#include <pnetv/jpn/analysis/StateSpace.h>

#include "PetriNet.h"

namespace jpna {

namespace {

inline jpn::Marking makeInitialMarking
  (std::unordered_map<we::place_id_type, TokenCount> const& token_count)
{
    std::vector<jpn::PlaceMarking> placeMarkings;
    for ( std::pair<we::place_id_type, TokenCount> token_count_on_place
        : token_count
        )
    {
      placeMarkings.emplace_back ( token_count_on_place.first
                                 , token_count_on_place.second
                                 );
    }
    return jpn::Marking(placeMarkings);
}

inline jpn::Marking makeMarking(const std::vector<we::place_id_type> &places) {
    std::vector<jpn::PlaceMarking> placeMarkings;
    for (we::place_id_type place_id : places) {
        placeMarkings.emplace_back (place_id, 1);
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

std::vector<const Transition *> makeTrace(const std::vector<we::transition_id_type> &trace, const PetriNet &petriNet) {
    std::vector<const Transition *> result;
    for (we::transition_id_type transitionId : trace) {
        result.emplace_back (petriNet.getTransition (transitionId));
    }
    return result;
}

} // anonymous namespace

VerificationResult verify(const PetriNet &petriNet) {
    jpn::Marking initialMarking = makeInitialMarking(petriNet.token_count());

    std::vector<jpn::Transition> transitions;

    for (const Transition *transition : petriNet.transitions()) {
        if (transition->conditionAlwaysTrue()) {
            transitions.emplace_back (makeTransition(transition));
        }
    }

    std::vector<we::transition_id_type> init;
    std::vector<we::transition_id_type> loop;
    if (jpn::analysis::findLoop(transitions, initialMarking, init, loop)) {
        return VerificationResult(VerificationResult::LOOPS, makeTrace(init, petriNet), makeTrace(loop, petriNet));
    }

    for (const Transition *transition : petriNet.transitions()) {
        if (!transition->conditionAlwaysTrue()) {
            transitions.emplace_back (makeTransition(transition));
        }
    }

    if (jpn::analysis::findLoop(transitions, initialMarking, init, loop)) {
        return VerificationResult(VerificationResult::MAYBE_LOOPS, makeTrace(init, petriNet), makeTrace(loop, petriNet));
    }

    return VerificationResult(VerificationResult::TERMINATES);
}

} // namespace jpna
