// mirko.rahn@itwm.fraunhofer.de
// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_UTIL_MAYBE_HPP
#define _FHG_UTIL_MAYBE_HPP 1

#include <fhg/util/show.hpp>

#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
    template<typename T>
    struct optional_hash
      : public std::unary_function<boost::optional<T>, std::size_t>
    {
      std::size_t operator() (boost::optional<T> const& x) const
      {
        return !x ? 0 : (1 + boost::hash<T>() (*x));
      }
    };

    template<typename T>
    std::ostream & operator << (std::ostream & s, const boost::optional<T> & x)
    {
      return s << (!x ? "Nothing" : ("Just " + fhg::util::show(*x)));
    };

    template<typename T, typename U>
    boost::optional<U> fmap (U (*f)(const T &), const boost::optional<T> & x)
    {
      return !x ? boost::none : boost::optional<U> (f (*x));
    }
  }
}

#endif
