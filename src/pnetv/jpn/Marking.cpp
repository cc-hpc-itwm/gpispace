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

#ifndef NDEBUG
    check();
#endif
}

#ifndef NDEBUG
void Marking::check() const {
    for (const PlaceMarking &placeMarking : placeMarkings_) {
        assert(placeMarking.count() != 0);
    }

    for (std::size_t i = 1; i < placeMarkings_.size(); ++i) {
        assert(placeMarkings_[i - 1].placeId() < placeMarkings_[i].placeId());
    }
}
#endif

} // namespace jpn
