// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_UTIL_HPP
#define FUSE_UTIL_HPP 1

#include <string>

namespace gpi_fuse
{
  namespace util
  {
    template<typename IT>
    static bool
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

    static bool
    starts_with (const std::string & p, const std::string & x)
    {
      return generic_starts_with (p.begin(), p.end(), x.begin(), x.end());
    }

    template<typename IT, typename U>
    static U
    generic_strip_prefix ( IT pos_p, const IT end_p
                         , IT pos_x, const IT end_x
                         )
    {
      while (pos_p != end_p)
        {
          if (pos_x == end_x)
            {
              throw std::runtime_error ("strip_prefix: prefix to long");
            }

          if (*pos_p != *pos_x)
            {
              throw std::runtime_error ("strip_prefix: wrong prefix");
            }

          ++pos_p;
          ++pos_x;
        }

      return U (pos_x, end_x);
    }

    static std::string strip_prefix ( const std::string & p
                                    , const std::string & x
                                    )
    {
      return generic_strip_prefix<std::string::const_iterator, std::string>
        (p.begin(), p.end(), x.begin(), x.end());
    }
  } // namespace util
} // namespace gpi_fuse

#endif
