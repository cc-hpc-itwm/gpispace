#pragma once

#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;
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

      template<typename Exception, typename T, typename... Args>
        T get_or_throw (boost::optional<T> const& optional, Args&&... args)
      {
        if (!optional)
        {
          throw Exception (args...);
        }

        return optional.get();
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
