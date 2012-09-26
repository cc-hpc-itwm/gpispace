#include "Marking.h"

#include <cassert>
#include <algorithm>
#include <iostream>
#include <ostream>

#include <fhg/assert.hpp>
#include <jpn/common/Foreach.h>
#include <jpn/common/PrintRange.h>

namespace jpn {

namespace {
    struct CountIsZero {
        bool operator()(const PlaceMarking &m) const {
            return m.count() == 0;
        }
    };

    struct CompareId {
        bool operator()(const PlaceMarking &a, const PlaceMarking &b) const {
            return a.placeId() < b.placeId();
        }
    };
} // anonymous namespace

Marking::Marking(const std::vector<PlaceMarking> &placeMarkings):
    placeMarkings_(placeMarkings)
{
    placeMarkings_.erase(std::remove_if(placeMarkings_.begin(), placeMarkings_.end(), CountIsZero()), placeMarkings_.end());
    std::sort(placeMarkings_.begin(), placeMarkings_.end(), CompareId());

#ifndef NDEBUG
    check();
#endif
}

void Marking::print(std::ostream &out) const {
    printRange(out, placeMarkings().begin(), placeMarkings().end());
}

#ifndef NDEBUG
void Marking::check() const {
    FOREACH (const PlaceMarking &placeMarking, placeMarkings_) {
        assert(placeMarking.count() != 0);
    }

    for (std::size_t i = 1; i < placeMarkings_.size(); ++i) {
        assert(placeMarkings_[i - 1].placeId() < placeMarkings_[i].placeId());
    }
}
#endif

} // namespace jpn

/* vim:set et sts=4 sw=4: */
