// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_UTIL_UNIQUE_HPP
#define _XML_UTIL_UNIQUE_HPP

#include <list>
#include <string>
#include <stdexcept>
#include <iterator>

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/optional.hpp>

#include <xml/parse/id/mapper.hpp>

namespace xml
{
  namespace util
  {
    template<typename T, typename ID_TYPE, typename KEY = std::string>
    class uniqueID
    {
    public:
      typedef T value_type;
      typedef ID_TYPE id_type;
      typedef KEY key_type;

      typedef boost::unordered_set<id_type> ids_type;
      typedef boost::unordered_map<key_type,id_type> by_key_type;

      uniqueID (parse::id::mapper* mapper)
        : _mapper (mapper)
        , _ids ()
        , _by_key ()
      {}

      boost::optional<const id_type&> get (const key_type& key) const
      {
        const typename by_key_type::const_iterator pos (_by_key.find (key));

        if (pos != _by_key.end())
          {
            return pos->second;
          }

        return boost::none;
      }

      const id_type& push (const id_type& id)
      {
        const key_type& name (_mapper->get (id)->name());

        boost::optional<const id_type&> id_old (get (name));

        if (id_old)
          {
            return *id_old;
          }

        _ids.insert (id);
        _by_key.insert (std::make_pair (name, id));

        return id;
      }

      const ids_type& ids() const { return _ids; }
      void clear() { _ids.clear(); _by_key.clear(); }

    private:
      parse::id::mapper* _mapper;
      ids_type _ids;
      by_key_type _by_key;
    };

    template<typename UNIQUE>
    class unique_iterator : public std::iterator< std::forward_iterator_tag
                                                , typename UNIQUE::value_type
                                                >
    {
      typedef UNIQUE unique_type;

      typedef typename unique_type::elements_type::iterator
        actual_iterator_type;
      actual_iterator_type _actual_iterator;

    public:
      unique_iterator(const actual_iterator_type& actual_iterator)
        : _actual_iterator (actual_iterator)
      { }

      unique_iterator& operator++()
      {
        ++_actual_iterator;
        return *this;
      }
      unique_iterator operator++ (int)
      {
        unique_iterator<UNIQUE> tmp (*this);
        operator++();
        return tmp;
      }
      bool operator== (const unique_iterator& rhs)
      {
        return _actual_iterator == rhs._actual_iterator;
      }
      bool operator!= (const unique_iterator& rhs)
      {
        return !(operator== (rhs));
      }
      typename UNIQUE::value_type& operator*() const
      {
        return *_actual_iterator;
      }
      typename UNIQUE::value_type* operator->() const
      {
        return &*_actual_iterator;
      }
    };

    template<typename UNIQUE>
    class const_unique_iterator
      : public std::iterator< std::forward_iterator_tag
                            , const typename UNIQUE::value_type
                            >
    {
      typedef UNIQUE unique_type;

      typedef typename unique_type::elements_type::const_iterator
        actual_iterator_type;
      actual_iterator_type _actual_iterator;

    public:
      const_unique_iterator(const actual_iterator_type& actual_iterator)
        : _actual_iterator (actual_iterator)
      { }

      const_unique_iterator& operator++()
      {
        ++_actual_iterator;
        return *this;
      }
      const_unique_iterator operator++ (int)
      {
        const_unique_iterator<UNIQUE> tmp (*this);
        operator++();
        return tmp;
      }
      bool operator== (const const_unique_iterator& rhs)
      {
        return _actual_iterator == rhs._actual_iterator;
      }
      bool operator!= (const const_unique_iterator& rhs)
      {
        return !(operator== (rhs));
      }
      const typename UNIQUE::value_type& operator*() const
      {
        return *_actual_iterator;
      }
      const typename UNIQUE::value_type* operator->() const
      {
        return &*_actual_iterator;
      }
    };

    template<typename T, typename ID_TYPE, typename Key = std::string>
    struct unique
    {
    public:
      typedef T value_type;
      typedef ID_TYPE id_type;
      typedef Key key_type;

      typedef std::list<value_type> elements_type;

      typedef unique_iterator<unique<value_type, id_type, key_type> >
        iterator;
      typedef const_unique_iterator<unique<value_type, id_type, key_type> >
        const_iterator;

    private:
      typedef boost::unordered_map< key_type
                                  , typename elements_type::iterator
                                  > names_type;

      typedef boost::unordered_map< id_type
                                  , typename elements_type::iterator
                                  > ids_type;

      elements_type _elements;
      names_type _names;
      ids_type _ids;

      inline value_type& insert (const value_type& x)
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
#define RETURN_PAIR_TYPE boost::optional<value_type&>, boost::optional<value_type>
      typedef std::pair<RETURN_PAIR_TYPE >
              push_return_type;

      push_return_type push_and_get_old_value (const value_type& x)
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

      boost::optional<value_type&> push (const value_type& x)
      {
        if (is_element (x.name()))
          {
            return boost::none;
          }

        return insert (x);
      }

      boost::optional<const value_type&>
      copy_by_key (const key_type & key) const
      {
        const typename names_type::const_iterator pos (_names.find (key));

        if (pos != _names.end())
        {
          return *(pos->second);
        }

        return boost::none;
      }

      bool is_element (const key_type & key) const
      {
        return _names.find (key) != _names.end();
      }

      void erase (const value_type& x)
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
  }
}

#endif
