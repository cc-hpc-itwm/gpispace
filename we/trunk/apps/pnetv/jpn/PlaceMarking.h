#pragma once

#include <jpn/config.h>
#include <jpn/Types.h>
#include <jpn/common/ExtendedInteger.h>
#include <jpn/common/Printable.h>

namespace jpn {

typedef ExtendedInteger<TokenCount> ExtendedTokenCount;

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

/* vim:set et sts=4 sw=4: */
