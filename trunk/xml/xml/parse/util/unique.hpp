// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_UTIL_UNIQUE_HPP
#define _XML_UTIL_UNIQUE_HPP

#include <vector>
#include <string>
#include <boost/unordered_map.hpp>

namespace xml
{
  namespace util
  {
    template<typename T, typename Key = std::string>
    struct unique
    {
    private:
      typedef std::vector<T> vec_type;
      typedef boost::unordered_map< Key
                                  , typename vec_type::size_type
                                  > map_type;

      vec_type vec;
      map_type names;
      typename vec_type::size_type pos;

    public:
      unique () : vec (), names (), pos (0) {}

      bool push (const T & x, T & old)
      {
        const typename map_type::const_iterator found (names.find (x.name));

        if (found != names.end())
          {
            old = vec[found->second];

            return false;
          }

        names[x.name] = pos++;
        vec.push_back (x);

        return true;
      }

      bool push (const T & x)
      {
        if (names.find (x.name) != names.end())
          {
            return false;
          }

        names[x.name] = pos++;
        vec.push_back (x);

        return true;
      }

      bool by_key (const Key & key, T & x) const
      {
        const typename map_type::const_iterator found (names.find (key));

        if (found == names.end())
          {
            return false;
          }

        x = vec[found->second];

        return true;
      }

      bool is_element (const Key & key) const
      {
        T x;

        return by_key (key, x);
      }

      void clear (void) { vec.clear(); names.clear(); pos = 0; }

      vec_type & elements (void) { return vec; }
      const vec_type & elements (void) const { return vec; }
    };
  }
}

#endif
