#pragma once

#include <jpn/config.h>

#include <vector>

#ifdef JPN_BOOST_HASH
#include <boost/functional/hash.hpp>
#endif

#include <fhg/assert.hpp>
#include <jpn/PlaceMarking.h>
#include <jpn/common/Foreach.h>

namespace jpn {

/**
 * Marking (of a Petri net).
 *
 * It is implemented as a vector of place markings.
 * All place markings in the vector have non-zero token count.
 * The vector is sorted by place ids.
 */
class Marking: public Printable {
    std::vector<PlaceMarking> placeMarkings_; ///< Markings of individual places.

    public:

    /**
     * Default constructor.
     */
    Marking() {}

    /**
     * Constructor from an array of individual place markings.
     *
     * Place markings with zero token count are removed.
     * Markings are sorted.
     *
     * \param[in] placeMarkings Markings of individual places.
     */
    Marking(const std::vector<PlaceMarking> &placeMarkings);

    /**
     * \return Markings of individual places.
     */
    const std::vector<PlaceMarking> &placeMarkings() const { return placeMarkings_; }

    virtual void print(std::ostream &out) const;

    private:

#ifndef NDEBUG
    /**
     * Check correctness of internal data structures.
     */
    void check() const;
#endif

    friend Marking operator+(const Marking &, const Marking &);
    friend Marking operator-(const Marking &, const Marking &);
#ifdef JPN_EXTENDED_MARKINGS
    friend Marking accelerate(const Marking &a, const Marking &b);
#endif
};

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return True iff the two markings are equal.
 */
inline
bool operator==(const Marking &a, const Marking &b) {
    return a.placeMarkings() == b.placeMarkings();
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return True iff the two markings are different.
 */
inline
bool operator!=(const Marking &a, const Marking &b) {
    return !(a == b);
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return True iff a is covered by b.
 */
inline
bool operator<=(const Marking &a, const Marking &b) {
    std::vector<PlaceMarking>::const_iterator j = b.placeMarkings().begin();
    std::vector<PlaceMarking>::const_iterator jend = b.placeMarkings().end();

    std::vector<PlaceMarking>::const_iterator iend = a.placeMarkings().end();
    for (std::vector<PlaceMarking>::const_iterator i = a.placeMarkings().begin(); i != iend; ++i) {
        while (j != jend && j->placeId() < i->placeId()) {
            ++j;
        }
        if (j == jend || j->placeId() != i->placeId() || j->count() < i->count()) {
            return false;
        }
    }

    return true;
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return True iff a is covered by b and they are not equal.
 */
inline
bool operator<(const Marking &a, const Marking &b) {
    return a <= b && a != b;
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return True iff b is covered by a.
 */
inline
bool operator>=(const Marking &a, const Marking &b) {
    return b <= a;
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return True iff b is covered by a and they are not equal.
 */
inline
bool operator>(const Marking &a, const Marking &b) {
    return b < a;
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return A marking which is a componentwise sum of the two.
 */
inline
Marking operator+(const Marking &a, const Marking &b) {
    Marking result;

    std::vector<PlaceMarking>::const_iterator i = a.placeMarkings().begin();
    std::vector<PlaceMarking>::const_iterator iend = a.placeMarkings().end();

    std::vector<PlaceMarking>::const_iterator j = b.placeMarkings().begin();
    std::vector<PlaceMarking>::const_iterator jend = b.placeMarkings().end();

    while (i != iend && j != jend) {
        if (i->placeId() < j->placeId()) {
            result.placeMarkings_.push_back(*i++);
        } else if (i->placeId() > j->placeId()) {
            result.placeMarkings_.push_back(*j++);
        } else {
            if (i->count() + j->count() != 0) {
                result.placeMarkings_.push_back(PlaceMarking(i->placeId(), i->count() + j->count()));
            }
            ++i;
            ++j;
        }
    }
    while (i != iend) {
        result.placeMarkings_.push_back(*i++);
    }
    while (j != jend) {
        result.placeMarkings_.push_back(*j++);
    }

#ifndef NDEBUG
    result.check();
#endif

    return result;
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return A marking which is a componentwise difference between a and b.
 */
inline
Marking operator-(const Marking &a, const Marking &b) {
    Marking result;

    std::vector<PlaceMarking>::const_iterator i = a.placeMarkings().begin();
    std::vector<PlaceMarking>::const_iterator iend = a.placeMarkings().end();

    std::vector<PlaceMarking>::const_iterator j = b.placeMarkings().begin();
    std::vector<PlaceMarking>::const_iterator jend = b.placeMarkings().end();

    while (i != iend && j != jend) {
        if (i->placeId() < j->placeId()) {
            result.placeMarkings_.push_back(*i);
            ++i;
        } else if (i->placeId() > j->placeId()) {
            result.placeMarkings_.push_back(PlaceMarking(j->placeId(), -j->count()));
            ++j;
        } else {
            if (i->count() - j->count() != 0) {
                result.placeMarkings_.push_back(PlaceMarking(i->placeId(), i->count() - j->count()));
            }
            ++i;
            ++j;
        }
    }
    while (i != iend) {
        result.placeMarkings_.push_back(*i++);
    }
    while (j != jend) {
        result.placeMarkings_.push_back(PlaceMarking(j->placeId(), -j->count()));
        ++j;
    }

#ifndef NDEBUG
    result.check();
#endif

    return result;
}

#ifdef JPN_EXTENDED_MARKINGS
/**
 * \param[in] a Marking.
 * \param[in] b Marking. a < b.
 *
 * \return A marking which is has an infinity for a given place p if a[p] < b[p] and b[p] otherwise.
 */
inline
Marking accelerate(const Marking &a, const Marking &b) {
    assert(a < b);

    Marking result;

    std::vector<PlaceMarking>::const_iterator i = a.placeMarkings().begin();
    std::vector<PlaceMarking>::const_iterator iend = a.placeMarkings().end();

    std::vector<PlaceMarking>::const_iterator j = b.placeMarkings().begin();
    std::vector<PlaceMarking>::const_iterator jend = b.placeMarkings().end();

    while (j != jend) {
        while (i != iend && i->placeId() < j->placeId()) {
            ++i;
        }

        if (i == iend || i->placeId() > j->placeId() || (i->placeId() == j->placeId() && i->count() < j->count())) {
            result.placeMarkings_.push_back(PlaceMarking(j->placeId(), ExtendedTokenCount(ExtendedTokenCount::PLUS_INF)));
        } else {
            result.placeMarkings_.push_back(*j);
        }

        ++j;
    }

    return result;
}
#endif

} // namespace jpn

#ifdef JPN_BOOST_HASH
namespace boost {
    template<>
    struct hash<jpn::Marking> {
        std::size_t operator()(const jpn::Marking &marking) const {
            std::size_t result = 0;
            foreach (const jpn::PlaceMarking &placeMarking, marking.placeMarkings()) {
                result ^= (result << 1) ^ hash<jpn::PlaceMarking>()(placeMarking);
            }
            return result;
        }
    };
}
#endif

/* vim:set et sts=4 sw=4: */
