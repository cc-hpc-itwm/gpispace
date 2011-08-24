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
      typedef std::list<T> elements_type;

    private:
      typedef boost::unordered_map< Key
                                  , typename elements_type::iterator
                                  > names_type;

      elements_type _elements;
      names_type _names;

    public:
      unique () : _elements (), _names () {}

      bool push (const T & x, T & old)
      {
        const typename names_type::const_iterator found (_names.find (x.name));

        if (found != _names.end())
          {
            old = *found->second;

            return false;
          }

        _names[x.name] = _elements.insert (_elements.end(), x);

        return true;
      }

      bool push (const T & x)
      {
        const typename names_type::const_iterator found (_names.find (x.name));

        if (found != _names.end())
          {
            return false;
          }

        _names[x.name] = _elements.insert (_elements.end(), x);

        return true;
      }

      bool by_key (const Key & key, T & x) const
      {
        const typename names_type::const_iterator found (_names.find (key));

        if (found == _names.end())
          {
            return false;
          }

        x = *found->second;

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

      void clear (void) { _elements.clear(); _names.clear(); }

      elements_type & elements (void) { return _elements; }
      const elements_type & elements (void) const { return _elements; }
    };
  }
}

#endif
