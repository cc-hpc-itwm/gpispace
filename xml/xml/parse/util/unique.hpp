// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_UNIQUE_HPP
#define _XML_PARSE_UTIL_UNIQUE_HPP

#include <xml/parse/id/mapper.hpp>

#include <string>
#include <stdexcept>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace xml
{
  namespace util
  {
    template<typename T, typename ID_TYPE, typename KEY = std::string>
    class unique
    {
    public:
      typedef T value_type;
      typedef ID_TYPE id_type;
      typedef KEY key_type;

      typedef boost::unordered_set<id_type> ids_type;
      typedef boost::unordered_map<key_type,id_type> by_key_type;

    private:
      struct indirect
      {
        indirect (parse::id::mapper* id_mapper)
          : _id_mapper (id_mapper)
        { }
        value_type& operator () (const id_type& id)
        {
          return *_id_mapper->get_ref (id);
        }
      private:
        parse::id::mapper* _id_mapper;
      };

      struct const_indirect
      {
        const_indirect (parse::id::mapper* id_mapper)
          : _id_mapper (id_mapper)
        { }
        const value_type& operator () (const id_type& id)
        {
          return *_id_mapper->get (id);
        }
      private:
        parse::id::mapper* _id_mapper;
      };

      struct values_type
      {
        typedef T value_type;

        typedef typename ids_type::iterator base_iterator;
        typedef typename ids_type::const_iterator const_base_iterator;
        typedef boost::function
          <value_type& (const typename base_iterator::value_type&)> fun_type;
        typedef boost::function
          <const value_type& (const typename base_iterator::value_type&)>
          const_fun_type;
        typedef boost::transform_iterator<fun_type, base_iterator>
          iterator;
        typedef boost::transform_iterator< const_fun_type
                                         , const_base_iterator
                                         > const_iterator;

        iterator begin()
        {
          return iterator (_ids.begin(), indirect (_mapper));
        }
        iterator end()
        {
          return iterator (_ids.end(), indirect (_mapper));
        }

        const_iterator begin() const
        {
          return const_iterator (_ids.begin(), const_indirect (_mapper));
        }
        const_iterator end() const
        {
          return const_iterator (_ids.end(), const_indirect (_mapper));
        }

        //! \note These are private, as only unique shall access them,
        //! but they need to be in here to work around boost ticket #5473.
        //! See c3fbafa and https://svn.boost.org/trac/boost/ticket/5473
      private:
        friend class unique<value_type, id_type, key_type>;

        values_type (parse::id::mapper* mapper)
          : _mapper (mapper)
          , _ids()
        { }

        parse::id::mapper* _mapper;
        ids_type _ids;
      };

    public:
      unique (parse::id::mapper* mapper)
        : _values (mapper)
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

      bool has (const key_type& key) const
      {
        return _by_key.find (key) != _by_key.end();
      }

      const id_type& push (const id_type& id)
      {
        const key_type& name (_values._mapper->get (id)->name());

        boost::optional<const id_type&> id_old (get (name));

        if (id_old)
          {
            return *id_old;
          }

        _values._ids.insert (id);
        _by_key.insert (std::make_pair (name, id));

        return id;
      }

      void erase (const id_type& id)
      {
        const typename ids_type::const_iterator it (_values._ids.find (id));
        if (it == _values._ids.end())
        {
          throw std::out_of_range ("unique::erase called with bad id");
        }
        _by_key.erase (id.get().name());
        _values._ids.erase (it);
      }

      const ids_type& ids() const { return _values._ids; }
      values_type& values() const { return _values; }

      void clear() { _values._ids.clear(); _by_key.clear(); }
      bool empty() const { return _by_key.empty(); }

    private:
      //! \note Needs to be mutable, as values() must return a
      //! non-const reference from a const method. Else, would need to
      //! const_cast this, which is ugly as well. See note in
      //! values_type.
      mutable values_type _values;

      by_key_type _by_key;
    };
  }
}

#endif
