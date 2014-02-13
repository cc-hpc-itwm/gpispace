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
      std::string::size_type split_pos_s = val.find (sep);
      std::string::size_type split_pos_e = split_pos_s + 1;

      const std::string first = val.substr(0, split_pos_s);
      if (split_pos_s != std::string::npos)
      {
        const std::string second = val.substr (split_pos_e);
        return std::make_pair(first, second);
      }
      else
      {
        return std::make_pair(first, "");
      }
    }

    inline
    std::pair<std::string, std::string>
    split (std::string const &val, char sep)
    {
      return split_string (val, sep);
    }

    template <typename OutputIterator>
    inline void split ( const std::string & s
                      , char sep
                      , OutputIterator out
                      )
    {
      if (s.empty())
        return;
      std::pair<std::string, std::string> h_t (split_string(s, sep));
      *out++ = h_t.first;
      split (h_t.second, sep, out);
    }
  }
}

#endif
