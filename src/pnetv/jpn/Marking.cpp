#include "Marking.h"

#include <fhg/assert.hpp>

#include <cassert>
#include <algorithm>

namespace jpn {

Marking::Marking(const std::vector<PlaceMarking> &placeMarkings):
    placeMarkings_(placeMarkings)
{
    placeMarkings_.erase
      ( std::remove_if
        ( placeMarkings_.begin(), placeMarkings_.end()
        , [](PlaceMarking const& m) { return m.count() == 0; }
        )
      , placeMarkings_.end()
      );
    std::sort
      ( placeMarkings_.begin(), placeMarkings_.end()
      , [](PlaceMarking const& a, PlaceMarking const& b)
      { return a.placeId() < b.placeId(); }
      );
}

} // namespace jpn
