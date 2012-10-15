// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_SPLIT_HPP
#define _FHG_UTIL_SPLIT_HPP

#include <iterator>
#include <string>

#include <boost/function.hpp>

namespace fhg
{
  namespace util
  {
    template<typename List>
    void lines (const std::string& s, const char sep, List& v)
    {
      std::string::const_iterator pos (s.begin());
      std::string::const_iterator item_begin (s.begin());
      std::string::const_iterator item_end (s.begin());
      const std::string::const_iterator& end (s.end());

      while (pos != end)
        {
          if (*pos == sep)
            {
              v.push_back (std::string (item_begin, item_end));
              ++pos;
              while (pos != end && (*pos == sep || isspace (*pos)))
                {
                  ++pos;
                }
              item_begin = item_end = pos;
            }
          else
            {
              ++item_end;
              ++pos;
            }
        }

      if (item_begin != item_end)
        {
          v.push_back (std::string (item_begin, item_end));
        }
    }

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
      std::string::size_type split_pos_s = val.find (sep);
      std::string::size_type split_pos_e = split_pos_s + sep.size ();

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
    split (std::string const &val, std::string const &sep)
    {
      return split_string (val, sep);
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
