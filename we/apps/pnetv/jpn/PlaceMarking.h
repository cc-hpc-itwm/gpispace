#pragma once

#include <jpn/config.h>

#ifdef JPN_BOOST_HASH
#include <boost/functional/hash.hpp>
#endif

#include <jpn/Types.h>
#ifdef JPN_EXTENDED_MARKINGS
#include <jpn/common/ExtendedInteger.h>
#endif
#include <jpn/common/Printable.h>

namespace jpn {

#ifdef JPN_EXTENDED_MARKINGS
typedef ExtendedInteger<TokenCount> ExtendedTokenCount;
#else
#define ExtendedTokenCount TokenCount
#endif

/**
 * Marking of a place.
 */
class PlaceMarking: public Printable {
    PlaceId placeId_; ///< Identifier of the place.
    ExtendedTokenCount count_; ///< Token count.

    public:

    /**
     * Class constructor.
     *
     * \param[in] placeId Identifier of the place.
     * \param[in] count Token count.
     */
    PlaceMarking(PlaceId placeId, ExtendedTokenCount count):
        placeId_(placeId), count_(count)
    {}

    /**
     * \return Identifier of the place.
     */
    PlaceId placeId() const { return placeId_; }

    /**
     * \return Token count.
     */
    const ExtendedTokenCount &count() const { return count_; }

    virtual void print(std::ostream &out) const;
};

/**
 * \param[in] a Place marking.
 * \param[in] b Place marking.
 *
 * \return True iff a and b are markings of the same place with the same token counts.
 */
inline
bool operator==(const PlaceMarking &a, const PlaceMarking &b) {
    return a.placeId() == b.placeId() && a.count() == b.count();
}

} // namespace jpn

#ifdef JPN_BOOST_HASH
namespace boost {
    template<>
    struct hash<jpn::PlaceMarking> {
        std::size_t operator()(const jpn::PlaceMarking &placeMarking) const {
            return placeMarking.placeId() << 8 | hash<jpn::ExtendedTokenCount>()(placeMarking.count());
        }
    };
}
#endif

#ifndef JPN_EXTENDED_MARKINGS
#undef ExtendedTokenCount
#endif

/* vim:set et sts=4 sw=4: */
