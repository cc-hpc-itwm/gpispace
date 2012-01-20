#include "PetriNet.h"

#include <jpn/common/Foreach.h>

namespace pneta {

void PetriNet::print(std::ostream &out) const {
    out << "digraph Blabla { label=\"" << name() << "\"; ";

    PlaceId placeId = 0;
    foreach (const Place &place, places()) {
        out << "place" << placeId << "[label=\"" << place.name() << " (" << place.initialMarking() << ")\",shape=\"ellipse\"];" << std::endl;
        ++placeId;
    }

    TransitionId transitionId = 0;
    foreach (const Transition &transition, transitions()) {
        out << "transition" << transitionId << "[label=\"" << transition.name() << "\",shape=\"box\"];" << std::endl;

        foreach (PlaceId placeId, transition.inputPlaces()) {
            out << "place" << placeId << " -> transition" << transitionId << std::endl;
        }

        foreach (PlaceId placeId, transition.outputPlaces()) {
            out << "transition" << transitionId << " -> place" << placeId << std::endl;
        }
        ++transitionId;
    }

    out << "}" << std::endl;
}

} // namespace pneta

/* vim:set et sts=4 sw=4: */
