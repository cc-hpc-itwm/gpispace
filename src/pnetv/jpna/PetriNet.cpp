#include "PetriNet.h"

#include <memory>

namespace jpna {

PetriNet::~PetriNet() {
    for (Transition *transition : transitions_) {
        delete transition;
    }
}

Transition *PetriNet::createTransition() {
    std::auto_ptr<Transition> result(new Transition(transitions_.size()));
    transitions_.push_back(result.get());
    return result.release();
}

  we::place_id_type PetriNet::createPlace ( TokenCount initial_marking
                                          )
  {
    we::place_id_type const place_id (_token_count.size());
    _token_count[place_id] = initial_marking;
    return place_id;
  }

} // namespace jpna
