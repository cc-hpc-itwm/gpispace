// mirko.rahn@itwm.fraunhofer.de

#ifndef FUSE_UTIL_HPP
#define FUSE_UTIL_HPP 1

#include <string>

#include <boost/optional.hpp>
#include <cctype>

namespace gpifs
{
  namespace util
  {
    template<typename IT>
    static inline bool
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

    static inline bool
    starts_with (const std::string & p, const std::string & x)
    {
      return generic_starts_with (p.begin(), p.end(), x.begin(), x.end());
    }

    template<typename IT, typename U>
    static inline U
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

    static inline std::string strip_prefix ( const std::string & p
                                           , const std::string & x
                                           )
    {
      return generic_strip_prefix<std::string::const_iterator, std::string>
        (p.begin(), p.end(), x.begin(), x.end());
    }

    namespace parse
    {
      template<typename IT>
      static inline boost::optional<std::string>
      require_rest ( std::string::const_iterator what_pos
                   , const std::string::const_iterator & what_end
                   , IT & pos
                   , const IT & end
                   )
      {
        while (what_pos != what_end)
          {
            if (pos == end || tolower (*pos) != tolower (*what_pos))
              {
                return boost::optional<std::string>
                  (std::string (what_pos, what_end));
              }

            ++what_pos;
            ++pos;
          }

        return boost::optional<std::string> (boost::none);
      }

      template<typename IT>
      static inline boost::optional<std::string>
      require_rest (const std::string & what, IT & pos, const IT & end)
      {
        return require_rest (what.begin(), what.end(), pos, end);
      }

      template<typename IT>
      static inline void skip_space (IT & pos, const IT & end)
      {
        while (pos != end && isspace (*pos))
          {
            ++pos;
          }
      }
    } // namespace parse
  } // namespace util
} // namespace gpifs

#endif
