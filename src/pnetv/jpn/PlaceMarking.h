#pragma once

#include <pnetv/jpn/Types.h>

namespace jpn {

/**
 * Marking of a place.
 */
class PlaceMarking {
    PlaceId placeId_; ///< Identifier of the place.
    TokenCount count_; ///< Token count.

    public:

    /**
     * Class constructor.
     *
     * \param[in] placeId Identifier of the place.
     * \param[in] count Token count.
     */
    PlaceMarking(PlaceId placeId, TokenCount count):
        placeId_(placeId), count_(count)
    {}

    /**
     * \return Identifier of the place.
     */
    PlaceId placeId() const { return placeId_; }

    /**
     * \return Token count.
     */
    const TokenCount &count() const { return count_; }
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
