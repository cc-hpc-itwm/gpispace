// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_SPLIT_HPP
#define _FHG_UTIL_SPLIT_HPP

#include <iterator>
#include <string>

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


    inline std::pair<std::string, std::string> split_string(const std::string &val
                                                           , const std::string &sep)
    {
      std::string::size_type split_pos = val.find(sep);

      const std::string first = val.substr(0, split_pos);
      if (split_pos != std::string::npos)
      {
        const std::string second = val.substr(split_pos+1);
        return std::make_pair(first, second);
      }
      else
      {
        return std::make_pair(first, "");
      }
    }

    template <typename OutputIterator>
    inline void split ( const std::string & s
                      , std::string const & sep
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
