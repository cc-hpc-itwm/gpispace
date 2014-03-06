// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_SPLIT_HPP
#define _FHG_UTIL_SPLIT_HPP

#include <iterator>
#include <utility> // std::pair
#include <string>

#include <boost/function.hpp>

#include <list>

namespace fhg
{
  namespace util
  {
    template<typename T, typename U>
      inline std::list<U>
    split ( const T & x
          , const typename std::iterator_traits<typename T::const_iterator>::value_type & s
          )
    {
      std::list<U> path;
      T key;

      for (typename T::const_iterator pos (x.begin()); pos != x.end(); ++pos)
      {
        if (*pos == s)
        {
          path.push_back (key);
          key.clear();
        }
        else
        {
          key.push_back (*pos);
        }
      }

      if (!key.empty())
      {
        path.push_back (key);
      }

      return path;
    }


    inline std::pair<std::string, std::string> split_string(const std::string &val
                                                           , char sep)
    {
      std::string::size_type const pos (val.find (sep));

      return std::make_pair
        ( val.substr (0, pos)
        , pos != std::string::npos ? val.substr (pos + 1) : ""
        );
    }
  }
}

#endif
