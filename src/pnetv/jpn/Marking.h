#pragma once

#include <pnetv/jpn/PlaceMarking.h>

#include <vector>

namespace jpn {

/**
 * Marking (of a Petri net).
 *
 * It is implemented as a vector of place markings.
 * All place markings in the vector have non-zero token count.
 * The vector is sorted by place ids.
 */
class Marking {
    std::vector<PlaceMarking> placeMarkings_; ///< Markings of individual places.

    public:

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
};

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return True iff the two markings are equal.
 */
inline bool operator== (const Marking &a, const Marking &b)
{
  return a.placeMarkings() == b.placeMarkings();
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return True iff the two markings are different.
 */
inline bool operator!= (const Marking &a, const Marking &b)
{
  return !(a == b);
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return True iff a is covered by b.
 */
inline bool operator<= (const Marking &a, const Marking &b)
{
  std::vector<PlaceMarking>::const_iterator j = b.placeMarkings().begin();
  std::vector<PlaceMarking>::const_iterator jend = b.placeMarkings().end();

  for (PlaceMarking const& i : a.placeMarkings())
  {
    while (j != jend && j->placeId() < i.placeId())
    {
      ++j;
    }
    if (j == jend || j->placeId() != i.placeId() || j->count() < i.count())
    {
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
inline bool operator< (const Marking &a, const Marking &b)
{
  return a <= b && a != b;
}

namespace
{
  inline Marking binop
    ( Marking const& a
    , Marking const& b
    , std::function<TokenCount (TokenCount, TokenCount)> op
    )
  {
    std::vector<PlaceMarking> result;

    std::vector<PlaceMarking>::const_iterator i = a.placeMarkings().begin();
    std::vector<PlaceMarking>::const_iterator iend = a.placeMarkings().end();

    std::vector<PlaceMarking>::const_iterator j = b.placeMarkings().begin();
    std::vector<PlaceMarking>::const_iterator jend = b.placeMarkings().end();

    while (i != iend && j != jend)
    {
      if (i->placeId() < j->placeId())
      {
        result.emplace_back (*i++);
      }
      else if (i->placeId() > j->placeId())
      {
        result.emplace_back (j->placeId(), op (0, j->count()));
        ++j;
      }
      else
      {
        result.emplace_back (i->placeId(), op (i->count(), j->count()));
        ++i;
        ++j;
      }
    }
    while (i != iend)
    {
      result.emplace_back (*i++);
    }
    while (j != jend)
    {
      result.emplace_back (j->placeId(), op (0, j->count()));
      ++j;
    }

    return {result};
  }
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return A marking which is a componentwise sum of the two.
 */
inline Marking operator+ (const Marking &a, const Marking &b)
{
  return binop (a, b, [](TokenCount l, TokenCount r) { return l + r; });
}

/**
 * \param[in] a Marking.
 * \param[in] b Marking.
 *
 * \return A marking which is a componentwise difference between a and b.
 */
inline Marking operator- (const Marking &a, const Marking &b)
{
  return binop (a, b, [](TokenCount l, TokenCount r) { return l - r; });
}
} // namespace jpn
