// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_HPP
#define _XML_PARSE_TYPE_TRANSITION_HPP

#include <xml/parse/types.hpp>

#include <xml/parse/util/maybe.hpp>
#include <xml/parse/util/unique.hpp>

#include <vector>

#include <iostream>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

#include <we/type/property.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      // ******************************************************************* //

      struct use_type
      {
        std::string name;
        int level;

        use_type (const std::string & _name, const int & _level) 
          : name (_name) 
          , level (_level)
        {}
      };

      std::ostream & operator << (std::ostream & s, const use_type & u)
      {
        return s << level (u.level) << "use (" << u.name << ")";
      }

      // ******************************************************************* //

      template<typename Fun>
      class transition_resolve : public boost::static_visitor<void>
      {
      private:
        const xml::parse::struct_t::set_type global;
        const state::type & state;
        const xml::parse::struct_t::forbidden_type & forbidden;

      public:
        transition_resolve 
        ( const xml::parse::struct_t::set_type & _global
        , const state::type & _state
        , const xml::parse::struct_t::forbidden_type & _forbidden
        )
          : global (_global)
          , state (_state)
          , forbidden (_forbidden)
        {}

        void operator () (use_type &) const { return; }
        void operator () (Fun & fun) const
        {
          fun.resolve (global, state, forbidden);
        }
      };

      // ******************************************************************* //

      template<typename Fun>
      class transition_type_check : public boost::static_visitor<void>
      {
      private:
        const state::type & state;

      public:
        transition_type_check (const state::type & _state) : state (_state) {}

        void operator () (const use_type &) const { return; }
        void operator () (const Fun & fun) const { fun.type_check (state); }
      };
      
      // ******************************************************************* //

      template<typename Net, typename Trans>
      class transition_get_function
        : public boost::static_visitor<function_type>
      {
      private:
        const Net & net;
        const Trans & trans;

      public:
        transition_get_function ( const Net & _net
                                , const Trans & _trans
                                )
          : net (_net)
          , trans (_trans)
        {}

        function_type operator () (const function_type & fun) const
        {
          return fun;
        }

        function_type operator () (const use_type & use) const
        {
          function_type fun;

          if (!net.get_function (use.name, fun))
            {
              throw error::unknown_function
                (use.name, trans.name, trans.path);
            }

          return fun;
        }
      };

      // ******************************************************************* //

      typedef std::vector<connect_type> connect_vec_type;

      struct transition_type
      {
      private:
        xml::util::unique<connect_type> _in;
        xml::util::unique<connect_type> _out;
        xml::util::unique<connect_type> _read;

      public:
        typedef boost::variant <function_type, use_type> f_type;
        
        f_type f;

        std::string name;
        boost::filesystem::path path;

        we::type::property::type prop;

        int level;

        xml::parse::struct_t::set_type structs_resolved;

        cond_vec_type cond;

        // ***************************************************************** //

        const connect_vec_type & in (void) const { return _in.elements(); }
        const connect_vec_type & out (void) const { return _out.elements(); }
        const connect_vec_type & read (void) const { return _read.elements(); }

        // ***************************************************************** //

        void push_in (const connect_type & connect)
        {
          if (!_in.push (connect))
            {
              throw error::duplicate_connect ("in", connect.name, name, path);
            }
        }

        void push_out (const connect_type & connect)
        {
          if (!_out.push (connect))
            {
              throw error::duplicate_connect ("out", connect.name, name, path);
            }
        }

        void push_read (const connect_type & connect)
        {
          if (!_read.push (connect))
            {
              throw error::duplicate_connect ("read", connect.name, name, path);
            }
        }

        // ***************************************************************** //

        void resolve ( const state::type & state
                     , const xml::parse::struct_t::forbidden_type & forbidden
                     )
        {
          const xml::parse::struct_t::set_type empty;

          resolve (empty, state, forbidden);
        }

        void resolve ( const xml::parse::struct_t::set_type & global
                     , const state::type & state
                     , const xml::parse::struct_t::forbidden_type & forbidden
                     )
        {
          boost::apply_visitor
            (transition_resolve<function_type> (global, state, forbidden), f);
        }

        // ***************************************************************** //

        template<typename Net>
        void type_check ( const std::string & direction
                        , const connect_type & connect
                        , const Net & net
                        , const state::type &
                        ) const
        {
          // existence of connect.place
          place_type place;

          if (!net.get_place (connect.place, place))
            {
              throw error::connect_to_nonexistent_place
                (direction, name, connect.place, path);
            }

          const function_type fun 
            ( boost::apply_visitor 
              (transition_get_function<Net, transition_type> (net, *this), f)
            );

          // existence of connect.port
          port_type port;

          const bool port_exists 
            ( (direction == "out") 
            ? fun.get_port_out (connect.port, port) 
            : fun.get_port_in (connect.port, port)
            );

          if (!port_exists)
            {
              throw error::connect_to_nonexistent_port
                (direction, name, connect.port, path);
            }

          // typecheck connect.place.type vs connect.port.type
          if (place.type != port.type)
            {
              throw error::connect_type_error<port_type, place_type>
                (direction, name, port, place, path);
            }
        };

        template<typename Net>
        void type_check (const Net & net, const state::type & state) const
        {
          // local checks
          for ( connect_vec_type::const_iterator connect (in().begin())
              ; connect != in().end()
              ; ++connect
              )
            {
              type_check ("in", *connect, net, state);
            }

          for ( connect_vec_type::const_iterator connect (read().begin())
              ; connect != read().end()
              ; ++connect
              )
            {
              type_check ("read", *connect, net, state);
            }

          for ( connect_vec_type::const_iterator connect (out().begin())
              ; connect != out().end()
              ; ++connect
              )
            {
              type_check ("out", *connect, net, state);
            }

          // recurs
          boost::apply_visitor ( transition_type_check<function_type> (state)
                               , f
                               );
        };
      };

      // ******************************************************************* //

      using petri_net::connection_t;
      using petri_net::PT;
      using petri_net::PT_READ;
      using petri_net::TP;

      template< typename Activity
              , typename Net
              , typename Trans
              , typename Fun
              , typename Map
              >
      void
      transition_synthesize
      ( const Trans & trans
      , const state::type & state
      , const Net & net
      , typename Activity::transition_type::net_type & we_net
      , const Map & pids
      , typename Activity::transition_type::edge_type & e
      ) 
      {
        typedef typename Activity::transition_type we_transition_type;
        typedef typename petri_net::tid_t tid_t;

        Fun fun
          ( boost::apply_visitor 
            ( transition_get_function<Net, Trans> (net, trans)
            , trans.f
            )
          );

        if (fun.name.isJust())
          {
            if (*fun.name != trans.name)
              {
                state.warn ( warning::overwrite_function_name_trans 
                             ( *fun.name
                             , fun.path
                             , trans.name
                             , trans.path
                             )
                           );
              }
          }

        fun.name = trans.name;

        fun.cond.insert ( fun.cond.end()
                        , trans.cond.begin()
                        , trans.cond.end()
                        );

        // WORK HERE: add prop from trans to fun

        we_transition_type we_trans 
          ( boost::apply_visitor
            ( function_synthesize<Activity, Net, Fun> (state, fun)
            , fun.f
            )
          );

        for ( connect_vec_type::const_iterator connect (trans.in().begin())
            ; connect != trans.in().end()
            ; ++connect
            )
          {
            we_trans.add_connections ()
              (get_pid (pids, connect->place), connect->port, connect->prop)
              ;
          }

        for ( connect_vec_type::const_iterator connect (trans.read().begin())
            ; connect != trans.read().end()
            ; ++connect
            )
          {
            we_trans.add_connections ()
              (get_pid (pids, connect->place), connect->port, connect->prop)
              ;
          }

        for ( connect_vec_type::const_iterator connect (trans.out().begin())
            ; connect != trans.out().end()
            ; ++connect
            )
          {
            we_trans.add_connections ()
              (connect->port, get_pid (pids, connect->place), connect->prop)
              ;
          }

        const tid_t tid (we_net.add_transition (we_trans));

        for ( connect_vec_type::const_iterator connect (trans.in().begin())
            ; connect != trans.in().end()
            ; ++connect
            )
          {
            we_net.add_edge 
              (e++, connection_t (PT, tid, get_pid (pids, connect->place)))
              ;
          }

        for ( connect_vec_type::const_iterator connect (trans.read().begin())
            ; connect != trans.read().end()
            ; ++connect
            )
          {
            we_net.add_edge
              (e++, connection_t (PT_READ, tid, get_pid (pids, connect->place)))
              ;
          }

        for ( connect_vec_type::const_iterator connect (trans.out().begin())
            ; connect != trans.out().end()
            ; ++connect
            )
          {
            we_net.add_edge
              (e++, connection_t (TP, tid, get_pid (pids, connect->place)))
              ;
          }

        return;
      };

      // ******************************************************************* //

      std::ostream & operator << (std::ostream & s, const transition_type & t)
      {
        s << level (t.level)     << "transition (" << std::endl;
        s << level (t.level + 1) << "name = " << t.name << std::endl;
        s << level (t.level + 1) << "path = " << t.path << std::endl;

        s << level(t.level+1) << "properties = " << std::endl;

        t.prop.writeTo (s, t.level+2);

        s << level (t.level + 1) << "connect-in = " << std::endl;

        for ( connect_vec_type::const_iterator pos (t.in().begin())
            ; pos != t.in().end()
            ; ++pos
            )
          {
            s << level (t.level + 2) << *pos << std::endl;
          }

        s << level (t.level + 1) << "connect-read = " << std::endl;

        for ( connect_vec_type::const_iterator pos (t.read().begin())
            ; pos != t.read().end()
            ; ++pos
            )
          {
            s << level (t.level + 2) << *pos << std::endl;
          }

        s << level (t.level + 1) << "connect-out = " << std::endl;

        for ( connect_vec_type::const_iterator pos (t.out().begin())
            ; pos != t.out().end()
            ; ++pos
            )
          {
            s << level (t.level + 2) << *pos << std::endl;
          }

        s << level(t.level+1) << "condition = " << std::endl;

        for ( cond_vec_type::const_iterator pos (t.cond.begin())
            ; pos != t.cond.end()
            ; ++pos
            )
          {
            s << level(t.level+2) << *pos << std::endl;
          }

        s << level (t.level + 1) << "def = " << std::endl;

        boost::apply_visitor (visitor::show (s), t.f);

        s << std::endl;

        return s << level (t.level) << ") // transition";
      }
    }
  }
}

#endif
