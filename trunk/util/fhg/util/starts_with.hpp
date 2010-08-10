// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_STARTS_WITH_HPP
#define _FHG_UTIL_STARTS_WITH_HPP 1

#include <string>

namespace fhg
{
  namespace util
  {
    template<typename IT>
    inline bool
    generic_starts_with ( IT pos_p, const IT end_p
                        , IT pos_x, const IT end_x
                        )
    {
      while (pos_p != end_p && pos_x != end_x)
        {
          if (*pos_p != *pos_x)
            {
              return false;
            }

          ++pos_p;
          ++pos_x;
        }

      if (pos_p == end_p)
        {
          return true;
        }

      return false;
    }

    inline bool
    starts_with (const std::string & p, const std::string & x)
    {
      return generic_starts_with (p.begin(), p.end(), x.begin(), x.end());
    }

    inline bool
    ends_with (const std::string & s, const std::string & x)
    {
      return generic_starts_with (s.rbegin(), s.rend(), x.rbegin(), x.rend());
    }
  }
}

#endif
