// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_PROPERTY_HPP
#define _WE_TYPE_PROPERTY_HPP

#include <we/type/property.fwd.hpp>

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>
#include <boost/optional.hpp>

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <stack>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>

#include <fhg/util/split.hpp>
#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

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

        class is_value : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const value_type &) const { return true; }
          template<typename T>
          bool operator () (const T &) const { return false; }
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
        const boost::optional<const value_type &>
        get_maybe_val (IT pos, IT end, IT zero) const
        {
          try
            {
              return get_val (pos, end, zero);
            }
          catch (const exception::missing_binding &)
            {
              return boost::none;
            }
          catch (const exception::not_a_val &)
            {
              return boost::none;
            }
        }

        // ----------------------------------------------------------------- //

      public:
        type () : map () {}

        const map_type & get_map (void) const { return map; }

        // ----------------------------------------------------------------- //

        template<typename IT>
        boost::optional<mapped_type> set ( IT pos
                                         , IT end
                                         , const value_type & val
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
              type t ( boost::apply_visitor ( visitor::mk_type<type>()
                                            , map[*pos]
                                            )
                     );

              old_value = t.set (pos + 1, end, val);

              map[*pos] = t;
            }

          return old_value;
        }

        boost::optional<mapped_type>
        set (const path_type & path, const value_type & val)
        {
          return set (path.begin(), path.end(), val);
        }

        boost::optional<mapped_type>
        set (const std::string & path, const value_type & val)
        {
          const path_type path_splitted (util::split (path));
          const boost::optional<mapped_type> ret (set (path_splitted, val));

          return ret;
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
          const path_type path_splitted (util::split (path));
          const mapped_type & ret (get (path_splitted));

          return ret;
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
          const path_type path_splitted (util::split (path));
          const value_type & ret (get_val (path_splitted));

          return ret;
        }

        // ----------------------------------------------------------------- //

        template<typename IT>
        const boost::optional<const value_type &>
        get_maybe_val (IT pos, IT end) const
        {
          return get_maybe_val (pos, end, pos);
        }

        const boost::optional<const value_type &>
        get_maybe_val (const path_type & path) const
        {
          return get_maybe_val (path.begin(), path.end());
        }

        const boost::optional<const value_type &>
        get_maybe_val (const std::string & path) const
        {
          const path_type path_splitted (util::split (path));
          const boost::optional<const value_type &> ret
            (get_maybe_val (path_splitted));

          return ret;
        }

        // ----------------------------------------------------------------- //

        template<typename IT>
        const value_type &
        get_with_default (IT pos, IT end, const value_type & dflt) const
        {
          return get_maybe_val (pos, end, pos).get_value_or (dflt);
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
          const path_type path_splitted (util::split (path));
          const value_type & ret (get_with_default (path_splitted, dflt));

          return ret;
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
          const path_type path_splitted (util::split (path));

          del (path_splitted);
        }
      };

      namespace dump
      {
        inline void dump ( xml_util::xmlstream &
                         , const type &
                         );

        namespace visitor
        {
          class dump : public boost::static_visitor<void>
          {
          private:
            xml_util::xmlstream & s;
            const key_type & key;

          public:
            dump (xml_util::xmlstream & _s, const key_type & _key)
              : s (_s)
              , key (_key)
            {}

            void operator () (const value_type & v) const
            {
              s.open ("property");
              s.attr ("key", key);
              s.content (v);
              s.close ();
            }

            template<typename T>
            void operator () (const T & x) const
            {
              s.open ("properties");
              s.attr ("name", key);

              ::we::type::property::dump::dump (s, x);

              s.close ();
            }
          };
        }

        inline void dump ( xml_util::xmlstream & s
                         , const type & p
                         )
        {
          for ( map_type::const_iterator pos (p.get_map().begin())
              ; pos != p.get_map().end()
              ; ++pos
              )
            {
              boost::apply_visitor ( visitor::dump (s, pos->first)
                                   , pos->second
                                   );
            }
        }
      }

      inline std::ostream & operator << (std::ostream & s, const type & t)
      {
        xml_util::xmlstream xs (s);

        dump::dump (xs, t);

        return s;
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

        inline stack_type dfs ( const type & t
                              , const path_type::const_iterator pre_pos
                              , const path_type::const_iterator pre_end
                              )
        {
          if (pre_pos == pre_end)
            {
              return dfs (t);
            }
          else
            {
              for ( map_type::const_iterator pos (t.get_map().begin())
                  ; pos != t.get_map().end()
                  ; ++pos
                  )
                {
                  if (pos->first == *pre_pos)
                    {
                      if ( boost::apply_visitor
                           ( we::type::property::visitor::is_value()
                           , pos->second
                           )
                         )
                        {
                          return stack_type();
                        }

                      return
                        dfs
                        ( boost::apply_visitor
                          ( we::type::property::visitor::get_map<const type &>()
                          , pos->second
                          )
                        , pre_pos + 1
                        , pre_end
                        )
                        ;
                    }
                }
            }

          return stack_type();
        }

        inline stack_type dfs (const type & t, const path_type & path)
        {
          return dfs (t, path.begin(), path.end());
        }

        inline stack_type dfs (const type & t, const std::string & path)
        {
          const path_type path_splitted (util::split (path));

          return dfs (t, path_splitted);
        }
      }
    }
  }
}

#endif
