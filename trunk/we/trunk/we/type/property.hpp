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

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>

#include <xml/parse/util/maybe.hpp>

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

        static path_type split (const std::string & s, const char & sep = '.')
        {
          path_type path;
          key_type key;

          for ( std::string::const_iterator pos (s.begin())
              ; pos != s.end()
              ; ++pos
              )
            {
              if (*pos == sep)
                {
                  path.push_back (key);
                  key.clear();
                }
              else
                {
                  key.push_back (*pos);
                }
            }
          path.push_back (key);

          return path;
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
          T operator () (T & t) const { return t; }

          template<typename V>
          T operator () (V &) const { return T(); }
        };

        template<typename T>
        class get_map : public boost::static_visitor<T>
        {
        public:
          T operator () (T & t) const { return t; }

          template<typename V>
          T operator () (V &) const
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
          const value_type & operator () (const T & v) const
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
        const maybe<value_type> get_maybe_val (IT pos, IT end, IT zero) const
        {
          try
            {
              return maybe<value_type> (get_val (pos, end, zero));
            }
          catch (const exception::missing_binding &)
            {
              return maybe<value_type>();
            }
          catch (const exception::not_a_val &)
            {
              return maybe<value_type>();
            }
        }

        // ----------------------------------------------------------------- //

      public:
        type () : map () {}

        // ----------------------------------------------------------------- //

        template<typename IT>
        bool set (IT pos, IT end, const value_type & val)
        {
          bool overwritten (false);

          if (pos == end)
            {
              throw exception::empty_path ("set");
            }

          if (std::distance (pos, end) == 1)
            {
              if (map.find (*pos) != map.end())
                {
                  overwritten = true;
                }

              map[*pos] = val;
            }
          else
            {
              type t ( boost::apply_visitor ( visitor::mk_type<type>()
                                            , map[*pos]
                                            )
                     );

              overwritten |= t.set (pos + 1, end, val);
              
              map[*pos] = t;
            }

          return overwritten;
        }

        bool set (const path_type & path, const value_type & val)
        {
          return set (path.begin(), path.end(), val);
        }

        bool set (const std::string & path, const value_type & val)
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
        const maybe<value_type> get_maybe_val (IT pos, IT end) const
        {
          return get_maybe_val (pos, end, pos);
        }

        const maybe<value_type> get_maybe_val (const path_type & path) const
        {
          return get_maybe_val (path.begin(), path.end());
        }

        const maybe<value_type> get_maybe_val (const std::string & path) const
        {
          return get_maybe_val (util::split (path));
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
    }
  }
}

#endif
