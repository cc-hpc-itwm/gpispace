// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_PROPERTY_HPP
#define _WE_TYPE_PROPERTY_HPP

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <stack>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>

#include <xml/parse/util/maybe.hpp>

#include <fhg/util/split.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      typedef std::string key_type;
      typedef std::string value_type;

      struct type;

      typedef boost::variant < boost::recursive_wrapper<type>
                             , value_type
                             > mapped_type;

      typedef std::vector<key_type> path_type;
      typedef boost::unordered_map<key_type, mapped_type> map_type;

      // ******************************************************************* //

      namespace util
      {
        static void level (std::ostream & s, const unsigned int l)
        {
          for (unsigned int i (0); i < l; ++i) { s << "  "; }
        }

        static path_type split (const key_type & s, const char & sep = '.')
        {
          return fhg::util::split<key_type, path_type> (s, sep);
        }
      }

      // ******************************************************************* //

      namespace exception
      {
        class missing_binding : public std::runtime_error
        {
        private:
          std::string nice ( path_type::const_iterator pos
                           , const path_type::const_iterator end
                           )
          {
            std::ostringstream s;

            s << "missing binding for ";

            for (; std::distance (pos, end) > 1; ++pos)
              {
                s << *pos << ".";
              }

            s << *pos;

            return s.str();
          }

        public:
          missing_binding ( path_type::const_iterator pos
                          , const path_type::const_iterator end
                          )
            : std::runtime_error (nice (pos, end))
          {}
        };

        class empty_path : public std::runtime_error
        {
        public:
          empty_path (const std::string & pre)
            : std::runtime_error (pre + ": empty path")
          {}
        };

        class not_a_map : public std::runtime_error
        {
        public:
          not_a_map (const std::string & pre)
            : std::runtime_error (pre + ": not a map")
          {}
        };

        class not_a_val : public std::runtime_error
        {
        public:
          not_a_val (const std::string & pre)
            : std::runtime_error (pre + ": not a val")
          {}
        };
      }

      // ******************************************************************* //

      namespace visitor
      {
        template<typename T>
        class mk_type : public boost::static_visitor<T>
        {
        public:
          T operator () (T const & t) const { return t; }

          template<typename V>
          T operator () (V) const { return T(); }
        };

        template<typename T>
        class get_map : public boost::static_visitor<T>
        {
        public:
          T operator () (T t) const { return t; }

          template<typename V>
          T operator () (V) const
          {
            throw exception::not_a_map ("visitor::get_map");
          }
        };

        class get_val : public boost::static_visitor<const value_type &>
        {
        public:
          const value_type & operator () (const value_type & v) const
          {
            return v;
          }

          template<typename T>
          const value_type & operator () (const T &) const
          {
            throw exception::not_a_val ("visitor::get_val");
          }
        };

        template<typename T>
        class show : public boost::static_visitor<>
        {
        private:
          std::ostream & s;
          unsigned int l;

        public:
          show ( std::ostream & _s
               , const unsigned int _l = 0
               )
            : s (_s)
            , l (_l)
          {}

          void operator () (const value_type & v) const
          {
            util::level(s, l); s << v << std::endl;
          }

          void operator () (const T & t) const
          {
            t.writeTo (s, l);
          }
        };
      }

      // ******************************************************************* //

      struct type
      {
      private:
        map_type map;

        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive & ar, const unsigned int)
        {
          ar & BOOST_SERIALIZATION_NVP(map);
        }

        // ----------------------------------------------------------------- //

        template<typename IT>
        const mapped_type & get (IT pos, IT end, IT zero) const
        {
          if (pos == end)
            {
              throw exception::empty_path ("get");
            }

          map_type::const_iterator map_pos (map.find (*pos));

          if (map_pos == map.end())
            {
              throw exception::missing_binding (zero, end);
            }

          if (std::distance (pos, end) == 1)
            {
              return map_pos->second;
            }
          else
            {
              const type & t
                ( boost::apply_visitor ( visitor::get_map<const type &>()
                                       , map_pos->second
                                       )
                );

              return t.get (pos + 1, end, zero);
            }
        }

        template<typename IT>
        const value_type & get_val (IT pos, IT end, IT zero) const
        {
          return boost::apply_visitor ( visitor::get_val()
                                      , get (pos, end, zero)
                                      );
        }

        template<typename IT>
        const maybe<const value_type &>
        get_maybe_val (IT pos, IT end, IT zero) const
        {
          try
            {
              return maybe<const value_type &> (get_val (pos, end, zero));
            }
          catch (const exception::missing_binding &)
            {
              return maybe<const value_type &>();
            }
          catch (const exception::not_a_val &)
            {
              return maybe<const value_type &>();
            }
        }

        // ----------------------------------------------------------------- //

      public:
        type () : map () {}

        const map_type & get_map (void) const { return map; }

        // ----------------------------------------------------------------- //

        template<typename IT>
        maybe<mapped_type> set (IT pos, IT end, const value_type & val)
        {
          maybe<mapped_type> old_value;

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
              type t ( boost::apply_visitor ( visitor::mk_type<type>()
                                            , map[*pos]
                                            )
                     );

              old_value = t.set (pos + 1, end, val);

              map[*pos] = t;
            }

          return old_value;
        }

        maybe<mapped_type>
        set (const path_type & path, const value_type & val)
        {
          return set (path.begin(), path.end(), val);
        }

        maybe<mapped_type>
        set (const std::string & path, const value_type & val)
        {
          return set (util::split (path), val);
        }

        // ----------------------------------------------------------------- //

        template<typename IT>
        const mapped_type & get (IT pos, IT end) const
        {
          return get (pos, end, pos);
        }

        const mapped_type & get (const path_type & path) const
        {
          return get (path.begin(), path.end());
        }

        const mapped_type & get (const std::string & path) const
        {
          return (get (util::split (path)));
        }

        // ----------------------------------------------------------------- //

        template<typename IT>
        const value_type & get_val (IT pos, IT end) const
        {
          return get_val (pos, end, pos);
        }

        const value_type & get_val (const path_type & path) const
        {
          return get_val (path.begin(), path.end());
        }

        const value_type & get_val (const std::string & path) const
        {
          return get_val (util::split (path));
        }

        // ----------------------------------------------------------------- //

        template<typename IT>
        const maybe<const value_type &>
        get_maybe_val (IT pos, IT end) const
        {
          return get_maybe_val (pos, end, pos);
        }

        const maybe<const value_type &>
        get_maybe_val (const path_type & path) const
        {
          return get_maybe_val (path.begin(), path.end());
        }

        const maybe<const value_type &>
        get_maybe_val (const std::string & path) const
        {
          return get_maybe_val (util::split (path));
        }

        // ----------------------------------------------------------------- //

        template<typename IT>
        const value_type &
        get_with_default (IT pos, IT end, const value_type & dflt) const
        {
          return get_maybe_val (pos, end, pos).get_with_default (dflt);
        }

        const value_type &
        get_with_default (const path_type & path, const value_type & dflt) const
        {
          return get_with_default (path.begin(), path.end(), dflt);
        }

        const value_type &
        get_with_default ( const std::string & path
                         , const value_type & dflt
                         ) const
        {
          return get_with_default (util::split (path), dflt);
        }

        // ----------------------------------------------------------------- //

        template<typename IT>
        void del (IT pos, IT end)
        {
          if (pos == end)
            {
              throw exception::empty_path ("del");
            }

          if (map.find (*pos) != map.end())
            {
              if (std::distance (pos, end) == 1)
                {
                  map.erase (*pos);
                }
              else
                {
                  type & t ( boost::apply_visitor ( visitor::get_map<type &>()
                                                  , map[*pos]
                                                  )
                           );

                  t.del (pos + 1, end);
                }
            }
        }

        void del (const path_type & path)
        {
          del (path.begin(), path.end());
        }

        void del (const std::string & path)
        {
          del (util::split (path));
        }

        // ----------------------------------------------------------------- //

        void writeTo (std::ostream & s, const unsigned int l) const
        {
          for ( map_type::const_iterator pos (map.begin())
              ; pos != map.end()
              ; ++pos
              )
            {
              util::level (s, l); s << pos->first << ":" << std::endl;

              boost::apply_visitor ( visitor::show<type> (s, l + 1)
                                   , pos->second
                                   );
            }
        }
      };

      inline std::ostream & operator << (std::ostream & s, const type & t)
      {
        t.writeTo (s, 1); return s;
      }

      // ******************************************************************* //

      namespace traverse
      {
        typedef std::pair<path_type, value_type> pair_type;
        typedef std::stack<pair_type> stack_type;

        namespace visitor
        {
          class dfs : public boost::static_visitor<void>
          {
          private:
            stack_type & stack;
            path_type & path;

          public:
            dfs (stack_type & _stack, path_type & _path)
              : stack (_stack)
              , path (_path)
            {}

            void operator () (const value_type & v) const
            {
              stack.push (std::make_pair (path, v));
            }

            void operator () (const type & t) const
            {
              for ( map_type::const_iterator pos (t.get_map().begin())
                  ; pos != t.get_map().end()
                  ; ++pos
                  )
                {
                  path.push_back (pos->first);

                  boost::apply_visitor (*this, pos->second);

                  path.pop_back ();
                }
            }
          };
        }

        inline stack_type dfs (const type & t)
        {
          stack_type stack;
          path_type path;

          for ( map_type::const_iterator pos (t.get_map().begin())
              ; pos != t.get_map().end()
              ; ++pos
              )
            {
              path.push_back (pos->first);

              boost::apply_visitor (visitor::dfs (stack, path), pos->second);

              path.pop_back ();
            }

          return stack;
        }
      }
    }
  }
}

#endif
