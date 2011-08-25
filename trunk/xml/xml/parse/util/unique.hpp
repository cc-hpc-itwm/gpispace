// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_UTIL_UNIQUE_HPP
#define _XML_UTIL_UNIQUE_HPP

#include <list>
#include <string>
#include <boost/unordered_map.hpp>

namespace xml
{
  namespace util
  {
    template<typename T, typename Key = std::string>
    struct unique
    {
    public:
      typedef std::vector<T> elements_type;

    private:
      typedef boost::unordered_map< Key
                                  , std::size_t
                                  > names_type;

      elements_type _elements;
      names_type _names;
      std::size_t _pos;

    public:
      unique () : _elements (), _names (), _pos (0) {}

      bool push (const T & x, T & old)
      {
        const typename names_type::const_iterator found (_names.find (x.name));

        if (found != _names.end())
          {
            old = _elements[found->second];

            return false;
          }

        _names[x.name] = _pos++;
        _elements.insert (_elements.end(), x);

        return true;
      }

      bool push (const T & x)
      {
        const typename names_type::const_iterator found (_names.find (x.name));

        if (found != _names.end())
          {
            return false;
          }

        _names[x.name] = _pos++;
        _elements.insert (_elements.end(), x);

        return true;
      }

      bool by_key (const Key & key, T & x) const
      {
        const typename names_type::const_iterator found (_names.find (key));

        if (found == _names.end())
          {
            return false;
          }

        x = _elements[found->second];

        return true;
      }

      bool is_element (const Key & key) const
      {
        const typename names_type::const_iterator found (_names.find (key));

        if (found == _names.end())
          {
            return false;
          }

        return true;
      }

      void clear (void) { _elements.clear(); _names.clear(); _pos = 0; }

      elements_type & elements (void) { return _elements; }
      const elements_type & elements (void) const { return _elements; }
    };
  }
}

#endif
