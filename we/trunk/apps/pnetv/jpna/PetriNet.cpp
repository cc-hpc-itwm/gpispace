#include "PetriNet.h"

#include <memory>

#include <jpn/common/Foreach.h>

namespace jpna {

PetriNet::~PetriNet() {
    foreach(Transition *transition, transitions_) {
        delete transition;
    }
    foreach(Place *place, places_) {
        delete place;
    }
}

Transition *PetriNet::createTransition() {
    std::auto_ptr<Transition> result(new Transition(transitions_.size()));
    transitions_.push_back(result.get());
    return result.release();
}

Place *PetriNet::createPlace() {
    std::auto_ptr<Place> result(new Place(places_.size()));
    places_.push_back(result.get());
    return result.release();
}

void PetriNet::print(std::ostream &out) const {
    out << "digraph Blabla { label=\"" << name() << "\"; ";

    foreach (const Place *place, places()) {
        out << "place" << place->id() << "[label=\"" << place->name() << " (" << place->initialMarking() << ")\",shape=\"ellipse\"];" << std::endl;
    }

    foreach (const Transition *transition, transitions()) {
        out << "transition" << transition->id() << "[label=\"" << transition->name() << "\",shape=\"box\"];" << std::endl;

        foreach (const Place *place, transition->inputPlaces()) {
            out << "place" << place->id() << " -> transition" << transition->id() << std::endl;
        }

        foreach (const Place *place, transition->outputPlaces()) {
            out << "transition" << transition->id() << " -> place" << place->id() << std::endl;
        }
    }

    out << "}" << std::endl;
}

} // namespace jpna

/* vim:set et sts=4 sw=4: */
