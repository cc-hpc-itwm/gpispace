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

      //! \todo This is not really nice, but the only way to do it,
      //! when wanting a copy of the old value back without pointer or
      //! default constructing.

      //! \note .first is new, .second is old.
#define RETURN_PAIR_TYPE boost::optional<T&>, boost::optional<T>
      typedef std::pair<RETURN_PAIR_TYPE >
              push_return_type;

      push_return_type push_and_get_old_value (const T& x)
      {
        const typename names_type::const_iterator pos (_names.find (x.name));

        if (pos != _names.end())
          {
            return std::make_pair<RETURN_PAIR_TYPE >
              (boost::none, *(pos->second));
          }

        return std::make_pair<RETURN_PAIR_TYPE >
          (insert (x), boost::none);
      }

#undef RETURN_PAIR_TYPE

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
