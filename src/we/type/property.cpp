// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <we/type/property.hpp>

#include <we/type/value/path/split.hpp>

#include <fhg/util/boost/variant.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/xml.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      typedef boost::unordered_map<key_type, mapped_type> map_type;

      namespace exception
      {
        empty_path::empty_path (const std::string& pre)
          : std::runtime_error (pre + ": empty path")
        {}

        not_a_map::not_a_map (const std::string& pre)
          : std::runtime_error (pre + ": not a map")
        {}
      }

      // ******************************************************************* //

      namespace
      {
        class mk_type : public boost::static_visitor<type>
        {
        public:
          type operator () (type const& t) const { return t; }

          template<typename V>
            type operator () (V) const { return type(); }
        };

        //! \note Templated, as it can be a type& or a const type&.
        template<typename T>
        class visitor_get_map : public boost::static_visitor<T>
        {
        public:
          T operator () (T t) const { return t; }

          template<typename V>
          T operator () (V) const
          {
            throw exception::not_a_map ("visitor_get_map");
          }
        };

        class is_value : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const value_type&) const { return true; }
          template<typename T>
          bool operator () (const T&) const { return false; }
        };
      }

      // ******************************************************************* //

      boost::optional<const mapped_type&>
        type::get ( const path_iterator& pos
                  , const path_iterator& end
                  , const path_iterator& zero
                  ) const
      {
        if (pos == end)
        {
          return boost::none;
        }

        map_type::const_iterator map_pos (map.find (*pos));

        if (map_pos == map.end())
        {
          return boost::none;
        }

        if (std::distance (pos, end) == 1)
        {
          return map_pos->second;
        }
        else
        {
          const type& t ( boost::apply_visitor ( visitor_get_map<const type&>()
                                               , map_pos->second
                                               )
                        );

          return t.get (boost::next (pos), end, zero);
        }
      }

      type::type () : map () {}

      list_type type::list() const
      {
        list_type l;

        typedef std::pair<key_type, mapped_type> key_mapped_type;

        BOOST_FOREACH (key_mapped_type const& key_mapped, map)
        {
          l.push_back (key_mapped);
        }

        return l;
      }

      // ----------------------------------------------------------------- //

      boost::optional<mapped_type> type::set ( const path_iterator& pos
                                             , const path_iterator& end
                                             , const value_type& val
                                             )
      {
        boost::optional<mapped_type> old_value;

        if (pos == end)
        {
          throw exception::empty_path ("set");
        }

        if (std::distance (pos, end) == 1)
        {
          map_type::const_iterator old_pos (map.find (*pos));

          if (old_pos != map.end())
          {
            old_value = old_pos->second;
          }

          map[*pos] = val;
        }
        else
        {
          type t (boost::apply_visitor (mk_type(), map[*pos]));

          old_value = t.set (boost::next (pos), end, val);

          map[*pos] = t;
        }

        return old_value;
      }

      boost::optional<mapped_type>
        type::set (const path_type& path, const value_type& val)
      {
        return set (path.begin(), path.end(), val);
      }

      boost::optional<mapped_type>
        type::set (const std::string& path, const value_type& val)
      {
        return set (pnet::type::value::path::split (path), val);
      }

      // ----------------------------------------------------------------- //

      const boost::optional<const value_type&> type::get
        (const path_iterator& pos, const path_iterator& end) const
      {
        boost::optional<const mapped_type&> mapped (get (pos, end, pos));

        if (!mapped)
        {
          return boost::none;
        }

        return fhg::util::boost::get_or_none<const value_type&> (*mapped);
      }

      const boost::optional<const value_type&>
        type::get (const path_type& path) const
      {
        return get (path.begin(), path.end());
      }

      const boost::optional<const value_type&>
        type::get (const std::string& path) const
      {
        return get (pnet::type::value::path::split (path));
      }

      namespace dump
      {
        namespace
        {
          class visitor_dump : public boost::static_visitor<void>
          {
          private:
            ::fhg::util::xml::xmlstream& s;
            const key_type& key;

          public:
            visitor_dump
              (::fhg::util::xml::xmlstream& _s, const key_type& _key)
              : s (_s)
              , key (_key)
            {}

            void operator () (const value_type& v) const
            {
              s.open ("property");
              s.attr ("key", key);
              s.content (v);
              s.close ();
            }

            void operator () (const type& x) const
            {
              s.open ("properties");
              s.attr ("name", key);

              ::we::type::property::dump::dump (s, x);

              s.close ();
            }
          };
        }

        void dump (::fhg::util::xml::xmlstream& s, const type& p)
        {
          BOOST_FOREACH (map_type::value_type const& pos, p.list())
          {
            boost::apply_visitor (visitor_dump (s, pos.first), pos.second);
          }
        }
      }

      std::ostream& operator << (std::ostream& s, const type& t)
      {
        ::fhg::util::xml::xmlstream xs (s);

        dump::dump (xs, t);

        return s;
      }

      // ******************************************************************* //

      namespace traverse
      {
        namespace
        {
          class visitor_dfs : public boost::static_visitor<void>
          {
          private:
            stack_type& stack;
            path_type& path;

          public:
            visitor_dfs (stack_type& _stack, path_type& _path)
              : stack (_stack)
              , path (_path)
            {}

            void operator () (const value_type& v) const
            {
              stack.push_back (std::make_pair (path, v));
            }

            void operator () (const type& t) const
            {
              BOOST_FOREACH (map_type::value_type const& pos, t.list())
              {
                path.push_back (pos.first);

                boost::apply_visitor (*this, pos.second);

                path.pop_back ();
              }
            }
          };
        }

        stack_type dfs (const type& t)
        {
          stack_type stack;
          path_type path;

          BOOST_FOREACH (map_type::value_type const& pos, t.list())
          {
            path.push_back (pos.first);

            boost::apply_visitor (visitor_dfs (stack, path), pos.second);

            path.pop_back ();
          }

          return stack;
        }
      }
    }
  }
}
