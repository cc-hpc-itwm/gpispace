#include "PlaceMarking.h"

#include <ostream>

namespace jpn {

void PlaceMarking::print(std::ostream &out) const {
    out << "(" << placeId() << "," << count() << ")";
}

} // namespace jpn

/* vim:set et sts=4 sw=4: */
