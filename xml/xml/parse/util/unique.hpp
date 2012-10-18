// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_UTIL_UNIQUE_HPP
#define _XML_UTIL_UNIQUE_HPP

#include <xml/parse/util/id_type.hpp>

#include <list>
#include <string>
#include <boost/unordered_map.hpp>
#include <boost/optional.hpp>

#include <stdexcept>

namespace xml
{
  namespace util
  {
    template<typename T, typename ID_TYPE, typename Key = std::string>
    struct unique
    {
    public:
      typedef std::list<T> elements_type;

    private:
      typedef boost::unordered_map< Key
                                  , typename elements_type::iterator
                                  > names_type;

      typedef boost::unordered_map< ID_TYPE
                                  , typename elements_type::iterator
                                  > ids_type;

      elements_type _elements;
      names_type _names;
      ids_type _ids;

      inline T& insert (const T& x)
      {
        typename elements_type::iterator
          pos (_elements.insert (_elements.end(), x));

        _names.insert (typename names_type::value_type (x.name(), pos));
        _ids.insert (typename ids_type::value_type (x.id(), pos));

        return *pos;
      }

    public:
      unique ()
        : _elements ()
        , _names ()
        , _ids ()
      {}
      unique (const unique & old)
        : _elements ()
        , _names ()
        , _ids ()
      { *this = old; }

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
        const typename names_type::const_iterator pos (_names.find (x.name()));

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
        if (is_element (x.name()))
          {
            return boost::none;
          }

        return insert (x);
      }

      boost::optional<T> copy_by_id (const ID_TYPE & id) const
      {
        const typename ids_type::const_iterator pos (_ids.find (id));

        if (pos != _ids.end())
        {
          return *(pos->second);
        }

        return boost::none;
      }

      //! \todo Most likely, there was never an intended difference
      //! between copy_by_key and ref_by_key. When rewriting
      //! copy_by_key, it was passing out a copy via a T& argument
      //! though. The only difference now is the name and returning an
      //! ?T or a ?T&. ?T& is only used in weaver of the editor and
      //! seems to not be needed.
      boost::optional<T> copy_by_key (const Key & key) const
      {
        const typename names_type::const_iterator pos (_names.find (key));

        if (pos != _names.end())
        {
          return *(pos->second);
        }

        return boost::none;
      }

      boost::optional<T&> ref_by_key (const Key & key) const
      {
        const typename names_type::const_iterator pos (_names.find (key));

        if (pos != _names.end())
          {
            return *(pos->second);
          }

        return boost::none;
      }

      bool is_element (const Key & key) const
      {
        return _names.find (key) != _names.end();
      }

      void erase (const T& x)
      {
        typename names_type::iterator pos (_names.find (x.name()));

        if (pos != _names.end())
        {
          _elements.erase (pos->second);
          _names.erase (pos);
          _ids.erase (_ids.find (x.id()));
        }
      }

      void clear (void)
      {
        _elements.clear();
        _names.clear();
        _ids.clear();
      }

      elements_type & elements (void) { return _elements; }
      const elements_type & elements (void) const { return _elements; }
    };


    template<typename T, typename ID_TYPE, typename Key = std::string>
    struct uniqueUU
    {
    public:
      typedef std::list<T> elements_type;

    private:
      typedef boost::unordered_map< Key
                                  , typename elements_type::iterator
                                  > names_type;

      typedef boost::unordered_map< ID_TYPE
                                  , typename elements_type::iterator
                                  > ids_type;

      elements_type _elements;
      names_type _names;
      ids_type _ids;

      inline T& insert (const T& x)
      {
        typename elements_type::iterator
          pos (_elements.insert (_elements.end(), x));

        _names.insert (typename names_type::value_type (x.nameUU(), pos));
        _ids.insert (typename ids_type::value_type (x.id(), pos));

        return *pos;
      }

    public:
      uniqueUU ()
        : _elements ()
        , _names ()
        , _ids ()
      {}
      uniqueUU (const uniqueUU & old)
        : _elements ()
        , _names ()
        , _ids ()
      { *this = old; }

      uniqueUU & operator = (const uniqueUU & other)
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
        const typename names_type::const_iterator pos (_names.find (x.nameUU()));

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
        if (is_element (x.nameUU()))
          {
            return boost::none;
          }

        return insert (x);
      }

      boost::optional<T> copy_by_id (const ID_TYPE & id) const
      {
        const typename ids_type::const_iterator pos (_ids.find (id));

        if (pos != _ids.end())
        {
          return *(pos->second);
        }

        return boost::none;
      }

      //! \todo Most likely, there was never an intended difference
      //! between copy_by_key and ref_by_key. When rewriting
      //! copy_by_key, it was passing out a copy via a T& argument
      //! though. The only difference now is the name and returning an
      //! ?T or a ?T&. ?T& is only used in weaver of the editor and
      //! seems to not be needed.
      boost::optional<T> copy_by_key (const Key & key) const
      {
        const typename names_type::const_iterator pos (_names.find (key));

        if (pos != _names.end())
        {
          return *(pos->second);
        }

        return boost::none;
      }

      boost::optional<T&> ref_by_key (const Key & key) const
      {
        const typename names_type::const_iterator pos (_names.find (key));

        if (pos != _names.end())
          {
            return *(pos->second);
          }

        return boost::none;
      }

      bool is_element (const Key & key) const
      {
        return _names.find (key) != _names.end();
      }

      void erase (const T& x)
      {
        typename names_type::iterator pos (_names.find (x.nameUU()));

        if (pos != _names.end())
        {
          _elements.erase (pos->second);
          _names.erase (pos);
          _ids.erase (_ids.find (x.id()));
        }
      }

      void clear (void)
      {
        _elements.clear();
        _names.clear();
        _ids.clear();
      }

      elements_type & elements (void) { return _elements; }
      const elements_type & elements (void) const { return _elements; }
    };
  }
}

#endif
