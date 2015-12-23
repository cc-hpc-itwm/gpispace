#include "PetriNet.h"

#include <memory>

#include <util-generic/cxx14/make_unique.hpp>

namespace jpna {

PetriNet::~PetriNet() {
    for (Transition *transition : transitions_) {
        delete transition;
    }
}

  Transition *PetriNet::createTransition ( std::string const& name
                                         , bool condition_always_true
                                         , we::priority_type priority
                                         )
  {
    std::unique_ptr<Transition> result
      (fhg::util::cxx14::make_unique<Transition>
                      ( transitions_.size()
                      , name
                      , condition_always_true
                      , priority
                      )
      );
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
