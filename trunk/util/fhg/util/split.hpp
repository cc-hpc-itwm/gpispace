// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_SPLIT_HPP
#define _FHG_UTIL_SPLIT_HPP

#include <iterator>

namespace fhg
{
  namespace util
  {
    template<typename T, typename VEC>
    inline VEC
    split ( const T & x
          , const typename std::iterator_traits<typename T::const_iterator>::value_type & s
          )
    {
      VEC path;
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

      path.push_back (key);

      return path;
    }
  }
}

#endif
