#include "PetriNet.h"

#include <memory>

namespace jpna {

PetriNet::~PetriNet() {
    for (Transition *transition : transitions_) {
        delete transition;
    }
    for (Place *place : places_) {
        delete place;
    }
}

Transition *PetriNet::createTransition() {
    std::auto_ptr<Transition> result(new Transition(transitions_.size()));
    transitions_.push_back(result.get());
    return result.release();
}

  Place *PetriNet::createPlace ( TokenCount initial_marking
                               )
  {
    std::auto_ptr<Place> result (new Place (places_.size()));
    _token_count[result->id()] = initial_marking;
    places_.push_back(result.get());
    return result.release();
  }

} // namespace jpna
