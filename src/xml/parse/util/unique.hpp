// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <string>
#include <stdexcept>

#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>
#include <boost/range/any_range.hpp>

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <set>

namespace fhg
{
  namespace pnet
  {
    namespace util
    {
      template<typename T>
        struct unique
      {
        using key_type = typename T::unique_key_type;

      private:
        struct
        {
          bool operator() (T const& lhs, T const& rhs)
          {
            return lhs.unique_key() < rhs.unique_key();
          }
        } _comparator;

        std::set<T, decltype (_comparator)> _values;

      public:
        using value_type = T;
        using const_iterator = typename decltype (_values)::const_iterator;

        template<typename DuplicateException>
          void push (value_type const& value)
        {
          auto const result (_values.emplace (value));
          if (!result.second)
          {
            throw DuplicateException (*result.first, value);
          }
        }

        boost::optional<T const&> get (key_type const& key) const
        {
          auto const it ( std::find_if ( _values.begin()
                                       , _values.end()
                                       , [&] (T const& value)
                                         {
                                           return value.unique_key() == key;
                                         }
                                       )
                        );

          if (it == _values.end())
          {
            return boost::none;
          }
          return *it;
        }

        bool has (key_type const& key) const
        {
          return !!get (key);
        }

        auto size() const -> decltype (_values.size()) { return _values.size(); }
        auto empty() const -> decltype (_values.empty()) { return _values.empty(); }
        auto begin() const -> decltype (_values.begin()) { return _values.begin(); }
        auto end() const -> decltype (_values.end()) { return _values.end(); }
      };
    }
  }
}

namespace xml
{
  //! \todo Missing namespace parse
  namespace util
  {
    template<typename VALUE_TYPE>
      using range_type = boost::any_range
                       < VALUE_TYPE
                       , boost::single_pass_traversal_tag
                       , VALUE_TYPE&
                       , std::ptrdiff_t
                       >;

    template<typename VALUE_TYPE, typename ID_TYPE>
    class unique
    {
    private:
      typedef VALUE_TYPE value_type;
      typedef ID_TYPE id_type;
      typedef typename value_type::unique_key_type key_type;

      typedef std::unordered_set<id_type> ids_type;
      typedef std::unordered_map<key_type,id_type> by_key_type;

      class values_type
      {
      public:
        typedef VALUE_TYPE value_type;

        typedef boost::transform_iterator
          < std::function<value_type& (const id_type&)>
          , typename ids_type::iterator
          > iterator;
        typedef boost::transform_iterator
          < std::function<const value_type& (const id_type&)>
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

        values_type() = default;

        ids_type _ids;
      };

    public:
      unique() = default;

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
        _by_key.emplace (key, id);

        return id;
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

      range_type<value_type> values()
      {
        return range_type<value_type> (_values.begin(), _values.end());
      }
      range_type<value_type const> values() const
      {
        return range_type<value_type const> (_values.begin(), _values.end());
      }

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
        for (const value_type& value : values())
        {
          copy.push (value.clone (parent, mapper));
        }
        return copy;
      }

      unique<value_type, id_type>& reparent
        (const typename value_type::parent_id_type& parent)
      {
        for (value_type& value : values())
        {
         value.parent (parent);
        }
        return *this;
      }

    private:
      values_type _values;
      by_key_type _by_key;
    };
  }
}
