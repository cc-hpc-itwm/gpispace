// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_TRANSITION_HPP
#define _XML_PARSE_TYPE_TRANSITION_HPP

#include <xml/parse/types.hpp>
#include <xml/parse/error.hpp>

#include <xml/parse/util/unique.hpp>
#include <xml/parse/util/property.hpp>
#include <xml/parse/util/weparse.hpp>
#include <xml/parse/util/validprefix.hpp>

#include <xml/parse/util/id_type.hpp>

#include <iostream>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

#include <we/type/property.hpp>
#include <we/type/id.hpp>

#include <fhg/util/maybe.hpp>
#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

#include <rewrite/validprefix.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
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

      template<typename Fun>
      class transition_distribute_function : public boost::static_visitor<void>
      {
      private:
        const state::type& _state;
        const functions_type& _functions;
        const templates_type& _templates;
        const specializes_type& _specializes;

      public:
        transition_distribute_function ( const state::type& state
                                       , const functions_type& functions
                                       , const templates_type& templates
                                       , const specializes_type& specializes
                                       )
          : _state (state)
          , _functions (functions)
          , _templates (templates)
          , _specializes (specializes)
        {}

        void operator () (use_type&) const { return; }
        void operator () (Fun& fun) const
        {
          fun.distribute_function ( _state
                                  , _functions
                                  , _templates
                                  , _specializes
                                  );
        }
      };

      // ******************************************************************* //

      template<typename Fun>
      class transition_sanity_check : public boost::static_visitor<void>
      {
      private:
        const state::type & state;

      public:
        transition_sanity_check (const state::type & _state) : state (_state) {}

        void operator () (const use_type &) const { return; }
        void operator () (const Fun & fun) const { fun.sanity_check (state); }
      };

      // ******************************************************************* //

      template<typename Fun>
      class transition_specialize : public boost::static_visitor<void>
      {
      private:
        const type::type_map_type & map;
        const type::type_get_type & get;
        const xml::parse::struct_t::set_type & known_structs;
        const state::type & state;

      public:
        transition_specialize
        ( const type::type_map_type & _map
        , const type::type_get_type & _get
        , const xml::parse::struct_t::set_type & _known_structs
        , const state::type & _state
        )
          : map (_map)
          , get (_get)
          , known_structs (_known_structs)
          , state (_state)
        {}

        void operator () (use_type &) const { return; }
        void operator () (Fun & fun) const
        {
          fun.specialize (map, get, known_structs, state);
        }
      };

      // ******************************************************************* //

      template<typename Net, typename Trans>
      class transition_get_function
        : public boost::static_visitor<function_type>
      {
      private:
        const Net & net;
        const state::type & state;
        const Trans & trans;

      public:
        transition_get_function ( const Net & _net
                                , const state::type & _state
                                , const Trans & _trans
                                )
          : net (_net)
          , state (_state)
          , trans (_trans)
        {}

        function_type operator () (const function_type & fun) const
        {
          return fun;
        }

        function_type operator () (const use_type & use) const
        {
          boost::optional<function_type> fun (net.get_function (use.name));

          if (!fun)
            {
              throw error::unknown_function
                (use.name, trans.name(), trans.path);
            }

          fun->name (trans.name());

          return *fun;
        }
      };

      // ******************************************************************* //

      typedef xml::util::unique<connect_type>::elements_type connections_type;

      struct transition_type
      {
      private:
        xml::util::unique<connect_type> _in;
        xml::util::unique<connect_type> _out;
        xml::util::unique<connect_type> _read;
        xml::util::unique<place_map_type> _place_map;

        id::transition _id;
        id::net _parent;

      public:
        typedef boost::variant <function_type, use_type>
                function_or_use_type;

        transition_type (const id::transition& id, const id::net& parent)
          : _id (id)
          , _parent (parent)
          , _function_or_use (boost::none)
        { }

        transition_type ( const function_or_use_type& function_or_use
                        , const id::transition& id
                        , const id::net& parent
                        )
          : _id (id)
          , _parent (parent)
          , _function_or_use (function_or_use)
        { }

        const id::transition& id() const
        {
          return _id;
        }

        const id::net& parent() const
        {
          return _parent;
        }

        bool is_same (const transition_type& other) const
        {
          return id() == other.id() && parent() == other.parent();
        }

      private:
        boost::optional<function_or_use_type> _function_or_use;

        std::string _name;

      public:
        const function_or_use_type& function_or_use() const
        {
          if (!_function_or_use)
          {
            throw std::runtime_error
              ("requested function or use with no function set!");
          }
          return *_function_or_use;
        }
        function_or_use_type& function_or_use()
        {
          if (!_function_or_use)
          {
            throw std::runtime_error
              ("requested function or use with no function set!");
          }
          return *_function_or_use;
        }
        const function_or_use_type& function_or_use
          (const function_or_use_type& function_or_use_)
        {
          return *(_function_or_use = function_or_use_);
        }

        const std::string& name() const
        {
          return _name;
        }
        const std::string& name(const std::string& name)
        {
          return _name = name;
        }

        boost::filesystem::path path;

        we::type::property::type prop;

        structs_type structs;

        conditions_type cond;

        requirements_type requirements;

        fhg::util::maybe<petri_net::prio_t> priority;

        fhg::util::maybe<bool> finline;

        fhg::util::maybe<bool> internal;

        // ***************************************************************** //

        const connections_type & in (void) const { return _in.elements(); }
        const connections_type & out (void) const { return _out.elements(); }
        const connections_type & read (void) const { return _read.elements(); }
        const place_maps_type & place_map (void) const
        {
          return _place_map.elements();
        }

        // ***************************************************************** //

        void push_in (const connect_type & connect)
        {
          if (!_in.push (connect))
            {
              throw error::duplicate_connect ("in", connect.name(), name(), path);
            }
        }

        void push_out (const connect_type & connect)
        {
          if (!_out.push (connect))
            {
              throw error::duplicate_connect ("out", connect.name(), name(), path);
            }
        }

        void push_inout (const connect_type& connect)
        {
          push_in (connect); push_out (connect);
        }

        void push_read (const connect_type & connect)
        {
          if (!_read.push (connect))
            {
              throw error::duplicate_connect ("read", connect.name(), name(), path);
            }
        }

        void push_place_map (const place_map_type & place_map)
        {
          if (!_place_map.push (place_map))
            {
              throw error::duplicate_place_map (place_map.name(), name(), path);
            }
        }

        // ***************************************************************** //

        void clear_ports ()
        {
          _in.clear();
          _out.clear();
          _read.clear();
        }

        void clear_place_map ()
        {
          _place_map.clear();
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
            ( transition_resolve<function_type> (global, state, forbidden)
            , function_or_use()
            );
        }

        // ***************************************************************** //

        void specialize ( const type::type_map_type & map
                        , const type::type_get_type & get
                        , const xml::parse::struct_t::set_type & known_structs
                        , const state::type & state
                        )
        {
          boost::apply_visitor
            ( transition_specialize<function_type> ( map
                                                   , get
                                                   , known_structs
                                                   , state
                                                   )
            , function_or_use()
            );
        }

        // ***************************************************************** //

        void sanity_check (const state::type & state) const
        {
          boost::apply_visitor ( transition_sanity_check<function_type> (state)
                               , function_or_use()
                               );
        }

        // ***************************************************************** //

        void distribute_function ( const state::type& state
                                 , const functions_type& functions
                                 , const templates_type& templates
                                 , const specializes_type& specializes
                                 )
        {
          boost::apply_visitor
            ( transition_distribute_function<function_type>
              ( state
              , functions
              , templates
              , specializes
              )
            , function_or_use()
            );
        }

        // ***************************************************************** //

        template<typename Net>
        void type_check ( const std::string & direction
                        , const connect_type & connect
                        , const Net & net
                        , const state::type & state
                        ) const
        {
          // existence of connect.place
          boost::optional<place_type> place (net.get_place (connect.place));

          if (!place)
            {
              throw error::connect_to_nonexistent_place
                (direction, name(), connect.place, path);
            }

          const function_type fun
            ( boost::apply_visitor
              (transition_get_function<Net, transition_type> ( net
                                                             , state
                                                             , *this
                                                             )
              , function_or_use()
              )
            );

          // existence of connect.port
          boost::optional<port_type> port ( (direction == "out")
                                          ? fun.get_port_out (connect.port)
                                          : fun.get_port_in (connect.port)
                                          );

          if (!port)
            {
              throw error::connect_to_nonexistent_port
                (direction, name(), connect.port, path);
            }

          // typecheck connect.place.type vs connect.port.type
          if (place->type != port->type)
            {
              throw error::connect_type_error ( direction
                                              , name()
                                              , *port
                                              , *place
                                              , path
                                              );
            }
        };

        template<typename Net>
        void type_check (const Net & net, const state::type & state) const
        {
          // local checks
          for ( connections_type::const_iterator connect (in().begin())
              ; connect != in().end()
              ; ++connect
              )
            {
              type_check ("in", *connect, net, state);
            }

          for ( connections_type::const_iterator connect (read().begin())
              ; connect != read().end()
              ; ++connect
              )
            {
              type_check ("read", *connect, net, state);
            }

          for ( connections_type::const_iterator connect (out().begin())
              ; connect != out().end()
              ; ++connect
              )
            {
              type_check ("out", *connect, net, state);
            }

          // recurs
          boost::apply_visitor ( transition_type_check<function_type> (state)
                               , function_or_use()
                               );
        };
      };

      // ******************************************************************* //

      using petri_net::connection_t;
      using petri_net::edge::PT;
      using petri_net::edge::PT_READ;
      using petri_net::edge::TP;

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
        typedef typename we_transition_type::expr_type we_expr_type;
        typedef typename we_transition_type::place_type we_place_type;
        typedef typename we_transition_type::place_type we_place_type;
        typedef typename we_transition_type::preparsed_cond_type we_cond_type;
        typedef typename petri_net::tid_t tid_t;

        if ((trans.in().size() == 0) && (trans.out().size() == 0))
          {
            state.warn
              ( warning::independent_transition ( trans.name()
                                                , state.file_in_progress()
                                                )
              );
          }

        Fun fun
          ( boost::apply_visitor
            ( transition_get_function<Net, Trans> (net, state, trans)
            , trans.function_or_use()
            )
          );

        for ( ports_type::const_iterator port_in (fun.in().begin())
            ; port_in != fun.in().end()
            ; ++port_in
            )
          {
            boost::optional<port_type> port_out
              (fun.get_port_out (port_in->name()));

            if (port_out && (port_out->type != port_in->type))
              {
                state.warn
                  ( warning::conflicting_port_types ( trans.name()
                                                    , port_in->name()
                                                    , port_in->type
                                                    , port_out->type
                                                    , state.file_in_progress()
                                                    )
                  );
              }
          }

        if (fun.name())
          {
            if (  (*fun.name() != trans.name())
               && (!rewrite::has_magic_prefix (trans.name()))
               )
              {
                state.warn ( warning::overwrite_function_name_trans
                             ( *fun.name()
                             , fun.path
                             , trans.name()
                             , trans.path
                             )
                           );
              }
          }

        fun.name (trans.name());

        if (fun.internal)
          {
            if (trans.internal && *trans.internal != *fun.internal)
              {
                state.warn ( warning::overwrite_function_internal_trans
                           ( trans.name()
                           , trans.path
                           )
                           );

                fun.internal = trans.internal;
              }
          }

        fun.cond.insert ( fun.cond.end()
                        , trans.cond.begin()
                        , trans.cond.end()
                        );

        fun.requirements.join (trans.requirements);

        util::property::join (state, fun.prop, trans.prop);

        if (  not trans.priority // WORK HERE: make it work with prio
           && (
               (  !state.synthesize_virtual_places()
               && !trans.place_map().empty()
               )
               || (  !state.no_inline()
                  && trans.finline.get_with_default(false)
                  && boost::apply_visitor (function_is_net(), fun.f)
                  )
              )
           )
          { // unfold

            // set a prefix
            const std::string prefix (rewrite::mk_prefix (trans.name()));
            const Net & net_old (boost::get<Net> (fun.f));
            const Net & net_new (set_prefix (net_old, prefix));

            place_map_map_type place_map_map;

            for ( place_maps_type::const_iterator
                    pm (trans.place_map().begin())
                ; pm != trans.place_map().end()
                ; ++pm
                )
              {
                const typename Map::const_iterator pid
                  (pids.find (pm->place_real));

                if (pid == pids.end())
                  {
                    throw
                      error::real_place_missing ( pm->place_virtual
                                                , pm->place_real
                                                , trans.name()
                                                , state.file_in_progress()
                                                );
                  }

                place_map_map[prefix + pm->place_virtual] = pid->second;
              }

            // synthesize into this level
            const Map pid_of_place
              ( net_synthesize<Activity, Net, Fun>
                ( we_net
                , place_map_map
                , net_new
                , state
                , e
                )
              );

            // go in the subnet
            const std::string cond_in (fun.condition());

            util::we_parser_t parsed_condition_in
              ( util::we_parse ( cond_in
                               , "condition"
                               , "unfold"
                               , trans.name()
                               , trans.path
                               )
              );

            we_transition_type trans_in
              ( prefix + "IN"
              , we_expr_type ()
              , we_cond_type (cond_in, parsed_condition_in)
              , true
              , fun.prop
              );

            for ( ports_type::const_iterator port (fun.in().begin())
                ; port != fun.in().end()
                ; ++port
                )
              {
                const signature::type type
                  (fun.type_of_port (we::type::PORT_IN, *port));

                trans_in.add_ports () ( port->name()
                                      , type
                                      , we::type::PORT_IN
                                      , port->prop
                                      );
                trans_in.add_ports () ( port->name()
                                      , type
                                      , we::type::PORT_OUT
                                      , port->prop
                                      );

                if (port->place)
                  {
                    trans_in.add_connections ()
                      ( port->name()
                      , get_pid (pid_of_place , prefix + *port->place)
                      , port->prop
                      )
                      ;
                  }
              }

            for ( connections_type::const_iterator connect (trans.in().begin())
                ; connect != trans.in().end()
                ; ++connect
                )
              {
                trans_in.add_connections ()
                  (get_pid (pids, connect->place), connect->port, connect->prop)
                  ;
              }

            for ( connections_type::const_iterator connect (trans.read().begin())
                ; connect != trans.read().end()
                ; ++connect
                )
              {
                trans_in.add_connections ()
                  (get_pid (pids, connect->place), connect->port, connect->prop)
                  ;
              }

            const tid_t tid_in (we_net.add_transition (trans_in));

            for ( ports_type::const_iterator port (fun.in().begin())
                ; port != fun.in().end()
                ; ++port
                )
              {
                if (port->place)
                  {
                    we_net.add_edge
                      ( e++, connection_t ( TP
                                          , tid_in
                                          , get_pid ( pid_of_place
                                                    , prefix + *port->place
                                                    )
                                          )
                      )
                      ;
                  }
              }

            for ( connections_type::const_iterator connect (trans.in().begin())
                ; connect != trans.in().end()
                ; ++connect
                )
              {
                we_net.add_edge
                  ( e++
                  , connection_t ( PT
                                 , tid_in
                                 , get_pid (pids, connect->place)
                                 )
                  )
                  ;
              }

            for ( connections_type::const_iterator connect (trans.read().begin())
                ; connect != trans.read().end()
                ; ++connect
                )
              {
                we_net.add_edge
                  ( e++, connection_t ( PT_READ
                                      , tid_in
                                      , get_pid (pids, connect->place)
                                      )
                  )
                  ;
              }

            // going out of the subnet
            const std::string cond_out ("true");

            util::we_parser_t parsed_condition_out
              ( util::we_parse ( cond_out
                               , "condition"
                               , "unfold"
                               , trans.name()
                               , trans.path
                               )
              );

            we_transition_type trans_out
              ( prefix + "OUT"
              , we_expr_type ()
              , we_cond_type (cond_out, parsed_condition_out)
              , true
              , fun.prop
              );

            for ( ports_type::const_iterator port (fun.out().begin())
                ; port != fun.out().end()
                ; ++port
                )
              {
                const signature::type type
                  (fun.type_of_port (we::type::PORT_OUT, *port));

                trans_out.add_ports () ( port->name()
                                       , type
                                       , we::type::PORT_IN
                                       , port->prop
                                       );
                trans_out.add_ports () ( port->name()
                                       , type
                                       , we::type::PORT_OUT
                                       , port->prop
                                       );

                if (port->place)
                  {
                    trans_out.add_connections ()
                      ( get_pid (pid_of_place , prefix + *port->place)
                      , port->name()
                      , port->prop
                      )
                      ;
                  }
              }

            std::size_t num_outport (0);

            for ( connections_type::const_iterator connect (trans.out().begin())
                ; connect != trans.out().end()
                ; ++connect, ++num_outport
                )
              {
                trans_out.add_connections ()
                  (connect->port, get_pid (pids, connect->place), connect->prop)
                  ;
              }

            if (num_outport > 1)
              {
                const std::string
                  key ("pnetc.warning.inline-many-output-ports");
                const boost::optional<const ::we::type::property::value_type&>
                  warning_switch (fun.prop.get_maybe_val (key));

                if (!warning_switch || *warning_switch != "off")
                  {
                    state.warn ( warning::inline_many_output_ports
                                 ( trans.name()
                                 , state.file_in_progress()
                                 )
                               );
                  }
              }

            const tid_t tid_out (we_net.add_transition (trans_out));

            for ( ports_type::const_iterator port (fun.out().begin())
                ; port != fun.out().end()
                ; ++port
                )
              {
                if (port->place)
                  {
                    we_net.add_edge
                      ( e++, connection_t ( PT
                                          , tid_out
                                          , get_pid ( pid_of_place
                                                    , prefix + *port->place
                                                    )
                                          )
                      )
                      ;
                    }
                }

            for ( connections_type::const_iterator connect (trans.out().begin())
                ; connect != trans.out().end()
                ; ++connect
                )
              {
                we_net.add_edge
                  ( e++
                  , connection_t ( TP
                                 , tid_out
                                 , get_pid (pids, connect->place)
                                 )
                  )
                  ;
              }
          } // unfold

        else

          { // not unfold

            // set the real-property
            for ( place_maps_type::const_iterator
                    pm (trans.place_map().begin())
                ; pm != trans.place_map().end()
                ; ++pm
                )
              {
                const typename Map::const_iterator pid
                  (pids.find (pm->place_real));

                if (pid == pids.end())
                  {
                    throw
                      error::real_place_missing ( pm->place_virtual
                                                , pm->place_real
                                                , trans.name()
                                                , state.file_in_progress()
                                                );
                  }

                we_place_type we_place (we_net.get_place (pid->second));

                std::ostringstream path;

                path << "real" << "." << trans.name();

                we_place.property().set (path.str(), pm->place_virtual);

                we_net.modify_place (pid->second, we_place);
              }

            we_transition_type we_trans
              ( boost::apply_visitor
                ( function_synthesize<Activity, Net, Fun> (state, fun)
                , fun.f
                )
              );

            for ( connections_type::const_iterator connect (trans.in().begin())
                ; connect != trans.in().end()
                ; ++connect
                )
              {
                we_trans.add_connections ()
                  (get_pid (pids, connect->place), connect->port, connect->prop)
                  ;
              }

            for ( connections_type::const_iterator connect (trans.read().begin())
                ; connect != trans.read().end()
                ; ++connect
                )
              {
                we_trans.add_connections ()
                  (get_pid (pids, connect->place), connect->port, connect->prop)
                  ;
              }

            for ( connections_type::const_iterator connect (trans.out().begin())
                ; connect != trans.out().end()
                ; ++connect
                )
              {
                we_trans.add_connections ()
                  (connect->port, get_pid (pids, connect->place), connect->prop)
                  ;
              }

            const tid_t tid (we_net.add_transition (we_trans));

            if (trans.priority)
              {
                we_net.set_transition_priority (tid, *trans.priority);
              }

            for ( connections_type::const_iterator connect (trans.in().begin())
                ; connect != trans.in().end()
                ; ++connect
                )
              {
                we_net.add_edge
                  (e++, connection_t (PT, tid, get_pid (pids, connect->place)))
                  ;
              }

            for ( connections_type::const_iterator connect (trans.read().begin())
                ; connect != trans.read().end()
                ; ++connect
                )
              {
                we_net.add_edge
                  (e++, connection_t (PT_READ, tid, get_pid (pids, connect->place)))
                  ;
              }

            for ( connections_type::const_iterator connect (trans.out().begin())
                ; connect != trans.out().end()
                ; ++connect
                )
              {
                const pid_t pid (get_pid (pids, connect->place));

                we_net.add_edge (e++, connection_t (TP, tid, pid));
              }
          } // not unfold

        return;
      };

      // ******************************************************************* //

      namespace dump
      {
        namespace visitor
        {
          class transition_dump : public boost::static_visitor<void>
          {
          private:
            xml_util::xmlstream & s;

          public:
            transition_dump (xml_util::xmlstream & _s) : s (_s) {}

            template<typename T>
            void operator () (const T & x) const
            {
              ::xml::parse::type::dump::dump (s, x);
            }
          };
        } // namespace visitor

        inline void dump ( xml_util::xmlstream & s
                         , const transition_type & t
                         )
        {
          s.open ("transition");
          s.attr ("name", t.name());
          s.attr ("priority", t.priority);
          s.attr ("inline", t.finline);
          s.attr ("internal", t.internal);

          ::we::type::property::dump::dump (s, t.prop);
          ::xml::parse::type::dump::dump (s, t.requirements);

          boost::apply_visitor (visitor::transition_dump (s), t.function_or_use());

          dumps (s, t.place_map().begin(), t.place_map().end());
          dumps (s, t.read().begin(), t.read().end(), "read");
          dumps (s, t.in().begin(), t.in().end(), "in");
          dumps (s, t.out().begin(), t.out().end(), "out");

          for ( conditions_type::const_iterator cond (t.cond.begin())
              ; cond != t.cond.end()
              ; ++cond
              )
            {
              s.open ("condition");
              s.content (*cond);
              s.close();
            }

          s.close ();
        }
      }
    }
  }
}

#endif
