// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_OPTIONAL_HPP
#define FHG_UTIL_BOOST_OPTIONAL_HPP

#include <fhg/util/show.hpp>

#include <boost/optional.hpp>

#include <ostream>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;

      template<typename T, typename U>
        boost::optional<U> fmap (U (*f)(const T &), const optional<T>& m)
      {
        if (m)
        {
          return f (*m);
        }
        else
        {
          return boost::none;
        }
      }

      template<typename T>
        void fmap (void (*f)(const T &), const optional<T> & m)
      {
        if (m)
        {
          f (*m);
        }
      }
    }
  }
}

namespace boost
{
  template<typename T>
  std::ostream& operator<< (std::ostream& s, const boost::optional<T>& x)
  {
    return x ? (s << "Just " << *x) : (s << "Nothing");
  }
}

#endif
