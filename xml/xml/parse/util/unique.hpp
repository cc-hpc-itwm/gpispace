// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_UTIL_UNIQUE_HPP
#define _XML_UTIL_UNIQUE_HPP

#include <list>
#include <string>
#include <boost/unordered_map.hpp>
#include <boost/optional.hpp>

#include <stdexcept>

namespace xml
{
  namespace util
  {
    template<typename T, typename Key = std::string, typename Hash = boost::hash<Key> >
    struct unique
    {
    public:
      typedef std::list<T> elements_type;

    private:
      typedef boost::unordered_map< Key
                                  , typename elements_type::iterator
                                  , Hash
                                  > names_type;

      elements_type _elements;
      names_type _names;

      inline T& insert (const T& x)
      {
        typename elements_type::iterator
          pos (_elements.insert (_elements.end(), x));

        _names.insert (typename names_type::value_type (x.name, pos));

        return *pos;
      }

    public:
      unique () : _elements (), _names () {}
      unique (const unique & old) : _elements (), _names () { *this = old; }

      unique & operator = (const unique & other)
      {
        if (this != &other)
          {
            clear();

            for ( typename elements_type::const_iterator
                    element (other._elements.begin())
                ; element != other._elements.end()
                ; ++element
                )
              {
                push (*element);
              }
          }

        return *this;
      }

      boost::optional<T&> push (const T& x, T& old)
      {
        const typename names_type::const_iterator pos (_names.find (x.name));

        if (pos != _names.end())
          {
            old = *(pos->second);

            return boost::none;
          }

        return insert (x);
      }

      boost::optional<T&> push (const T& x)
      {
        if (is_element (x.name))
          {
            return boost::none;
          }

        return insert (x);
      }

      bool by_key (const Key & key, T & x) const
      {
        const typename names_type::const_iterator pos (_names.find (key));

        if (pos == _names.end())
          {
            return false;
          }

        x = *(pos->second);

        return true;
      }

      bool is_element (const Key & key) const
      {
        return _names.find (key) != _names.end();
      }

      boost::optional<T&> by_key (const Key & key)
      {
        typename names_type::iterator pos (_names.find (key));

        if (pos != _names.end())
          {
            return *(pos->second);
          }

        return boost::none;
      }

      void erase (const T& x)
      {
        typename names_type::iterator pos (_names.find (x.name));

        if (pos != _names.end())
          {
            _elements.erase (pos->second);
            _names.erase (pos);
          }
      }

      void clear (void) { _elements.clear(); _names.clear(); }

      elements_type & elements (void) { return _elements; }
      const elements_type & elements (void) const { return _elements; }
    };
  }
}

#endif
