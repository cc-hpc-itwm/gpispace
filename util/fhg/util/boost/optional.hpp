// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_OPTIONAL_HPP
#define FHG_UTIL_BOOST_OPTIONAL_HPP

#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      template<typename T, typename U>
      boost::optional<U> fmap (U (*f)(const T &), const boost::optional<T>& m)
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
