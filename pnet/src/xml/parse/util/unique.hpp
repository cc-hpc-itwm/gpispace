// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_UNIQUE_HPP
#define _XML_PARSE_UTIL_UNIQUE_HPP

#include <string>
#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace xml
{
  //! \todo Missing namespace parse
  namespace util
  {
    template<typename VALUE_TYPE, typename ID_TYPE>
    class unique
    {
    private:
      typedef VALUE_TYPE value_type;
      typedef ID_TYPE id_type;
      typedef typename value_type::unique_key_type key_type;

      typedef boost::unordered_set<id_type> ids_type;
      typedef boost::unordered_map<key_type,id_type> by_key_type;

      class values_type
      {
      public:
        typedef VALUE_TYPE value_type;

        typedef boost::transform_iterator
          < boost::function<value_type& (const id_type&)>
          , typename ids_type::iterator
          > iterator;
        typedef boost::transform_iterator
          < boost::function<const value_type& (const id_type&)>
          , typename ids_type::const_iterator
          > const_iterator;

        iterator begin()
        {
          return iterator (_ids.begin(), &id_type::get_ref);
        }
        iterator end()
        {
          return iterator (_ids.end(), &id_type::get_ref);
        }

        const_iterator begin() const
        {
          return const_iterator (_ids.begin(), &id_type::get);
        }
        const_iterator end() const
        {
          return const_iterator (_ids.end(), &id_type::get);
        }

        //! \note These are private, as only unique shall access them,
        //! but they need to be in here to work around boost ticket #5473.
        //! See c3fbafa and https://svn.boost.org/trac/boost/ticket/5473
      private:
        friend class unique<value_type, id_type>;

        values_type()
          : _ids()
        { }

        ids_type _ids;
      };

    public:
      unique()
        : _values()
        , _by_key()
      {}

      unique ( const unique<value_type, id_type>& other
             , const typename value_type::parent_id_type& parent
             )
        : _values (other._values)
        , _by_key (other._by_key)
      {
        reparent (parent);
      }

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
      bool has (const id_type& id) const
      {
        return _values._ids.find (id) != _values._ids.end();
      }

      const id_type& push (const id_type& id)
      {
        const key_type& key (id.get().unique_key());

        boost::optional<const id_type&> id_old (get (key));

        if (id_old)
          {
            return *id_old;
          }

        _values._ids.insert (id);
        _by_key.insert (std::make_pair (key, id));

        return id;
      }

      void push (const unique<value_type, id_type>& other, const std::string& m)
      {
        BOOST_FOREACH (const id_type& id, other.ids())
        {
          if (push (id) != id)
          {
            throw std::runtime_error ("non-unique join: " + m);
          }
        }
      }

      void erase (const id_type& id)
      {
        const typename ids_type::const_iterator it (_values._ids.find (id));
        if (it == _values._ids.end())
        {
          throw std::out_of_range ("unique::erase called with bad id");
        }
        _by_key.erase (id.get().unique_key());
        _values._ids.erase (it);
      }

      const ids_type& ids() const { return _values._ids; }
      values_type& values() const { return _values; }

      void clear() { _values._ids.clear(); _by_key.clear(); }
      bool empty() const { return _by_key.empty(); }

      unique<value_type, id_type> clone
        ( const boost::optional<typename value_type::parent_id_type>& parent
        = boost::none
        , const boost::optional<parse::id::mapper*>& mapper = boost::none
        ) const
      {
        //! \todo Reserve?
        unique<value_type, id_type> copy;
        BOOST_FOREACH (const value_type& value, values())
        {
          copy.push (value.clone (parent, mapper));
        }
        return copy;
      }

      unique<value_type, id_type>& reparent
        (const typename value_type::parent_id_type& parent)
      {
        BOOST_FOREACH (value_type& value, values())
        {
         value.parent (parent);
        }
        return *this;
      }

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
