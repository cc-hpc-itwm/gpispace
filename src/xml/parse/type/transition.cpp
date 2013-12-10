// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/transition.hpp>

#include <xml/parse/id/mapper.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/use.hpp>
#include <xml/parse/util/property.hpp>
#include <xml/parse/util/weparse.hpp>

#include <xml/parse/type/dumps.hpp>

#include <we/type/place.hpp>
#include <we/type/expression.hpp>

#include <boost/variant.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      transition_type::transition_type
        ( ID_CONS_PARAM(transition)
        , PARENT_CONS_PARAM(net)
        , const util::position_type& pod
        , const std::string& name
        , const boost::optional<petri_net::priority_type>& priority
        , const boost::optional<bool>& finline
        , const boost::optional<bool>& internal
        )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _name (name)
        , priority (priority)
        , finline (finline)
        , internal (internal)
      {
        _id_mapper->put (_id, *this);
      }

      namespace
      {
        typedef transition_type::function_or_use_type function_or_use_type;

        class visitor_reparent : public boost::static_visitor<void>
        {
        public:
          visitor_reparent (const id::transition& parent)
            : _parent (parent)
          { }

          void operator() (const id::ref::function& id) const
          {
            id.get_ref().parent (_parent);
          }
          void operator() (const id::ref::use& id) const
          {
            id.get_ref().parent (_parent);
          }

        private:
          const id::transition& _parent;
        };

        const function_or_use_type& reparent ( const function_or_use_type& fun
                                             , const id::transition& parent
                                             )
        {
          boost::apply_visitor (visitor_reparent (parent), fun);
          return fun;
        }
      }

      transition_type::transition_type
        ( ID_CONS_PARAM(transition)
        , PARENT_CONS_PARAM(net)
        , const util::position_type& pod
        , const function_or_use_type& function_or_use
        )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _function_or_use (reparent (function_or_use, _id))
      {
        _id_mapper->put (_id, *this);
      }

      transition_type::transition_type
        ( ID_CONS_PARAM(transition)
        , PARENT_CONS_PARAM(net)
        , const util::position_type& pod
        , const boost::optional<function_or_use_type>& fun_or_use
        , const std::string& name
        , const connections_type& connections
        , const place_maps_type& place_map
        , const structs_type& structs
        , const conditions_type& conditions
        , const requirements_type& requirements
        , const boost::optional<petri_net::priority_type>& priority
        , const boost::optional<bool>& finline
        , const boost::optional<bool>& internal
        , const we::type::property::type& properties
        )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _function_or_use ( fun_or_use
                           ? boost::make_optional (reparent (*fun_or_use, _id))
                           : boost::none
                           )
        , _name (name)
        , _connections (connections, _id)
        , _place_map (place_map, _id)
        , structs (structs)
        , _conditions (conditions)
        , requirements (requirements)
        , priority (priority)
        , finline (finline)
        , internal (internal)
        , _properties (properties)
      {
        _id_mapper->put (_id, *this);
      }

      const transition_type::function_or_use_type&
        transition_type::function_or_use() const
      {
        if (!_function_or_use)
        {
          throw std::runtime_error
            ("requested function or use with no function set!");
        }
        return *_function_or_use;
      }
      transition_type::function_or_use_type&
        transition_type::function_or_use()
      {
        if (!_function_or_use)
        {
          throw std::runtime_error
            ("requested function or use with no function set!");
        }
        return *_function_or_use;
      }
      const transition_type::function_or_use_type&
        transition_type::function_or_use
        (const function_or_use_type& function_or_use_)
      {
        return *(_function_or_use = reparent (function_or_use_, id()));
      }

      namespace
      {
        class transition_get_function
          : public boost::static_visitor <const id::ref::function&>
        {
        private:
          const net_type & net;
          const transition_type & trans;

        public:
          transition_get_function ( const net_type & _net
                                  , const transition_type & _trans
                                  )
            : net (_net)
            , trans (_trans)
          {}

          const id::ref::function&
          operator () (const id::ref::function& id_function) const
          {
            return id_function;
          }

          const id::ref::function&
            operator() (const id::ref::use& use) const
          {
            boost::optional<const id::ref::function&>
              id_function (net.get_function (use.get().name()));

            if (not id_function)
            {
              throw error::unknown_function
                (use.get().name(), trans.make_reference_id());
            }

            return *id_function;
          }
        };
      }

      id::ref::function transition_type::resolved_function() const
      {
        return boost::apply_visitor
          (transition_get_function (*parent(), *this), function_or_use());
      }

      namespace
      {
        template<typename T>
          class is_of_type_visitor : public boost::static_visitor<bool>
        {
        public:
          bool operator() (const T&) const
          {
            return true;
          }
          template<typename U>
            bool operator() (const U&) const
          {
            return false;
          }
        };

        template<typename T, typename V>
          bool is_of_type (const V& variant)
        {
          return boost::apply_visitor (is_of_type_visitor<T>(), variant);
        }
      }

      const std::string& transition_type::name() const
      {
        return _name;
      }
      const std::string& transition_type::name_impl (const std::string& name)
      {
        return _name = name;
      }

      const std::string& transition_type::name (const std::string& name)
      {
        if (has_parent())
        {
          parent()->rename (make_reference_id(), name);
          return _name;
        }

        return name_impl (name);
      }

      const transition_type::connections_type&
        transition_type::connections() const
      {
        return _connections;
      }
      const transition_type::place_maps_type&
        transition_type::place_map() const
      {
        return _place_map;
      }

      boost::optional<const id::ref::function&>
      transition_type::get_function (const std::string& name) const
      {
        if (has_parent())
          {
            return parent()->get_function (name);
          }

        return boost::none;
      }

      // ***************************************************************** //
      void transition_type::remove_connection (const id::ref::connect& id)
      {
        _connections.erase (id);
      }
      void transition_type::remove_place_map (const id::ref::place_map& id)
      {
        _place_map.erase (id);
      }

      void transition_type::push_connection (const id::ref::connect& connect_id)
      {
        const id::ref::connect& id_old (_connections.push (connect_id));

        if (not (id_old == connect_id))
        {
          throw error::duplicate_connect (id_old, connect_id);
        }
        connect_id.get_ref().parent (id());
      }

      void transition_type::push_place_map (const id::ref::place_map& pm_id)
      {
        const id::ref::place_map& id_old (_place_map.push (pm_id));

        if (not (id_old == pm_id))
        {
          throw error::duplicate_place_map (id_old, pm_id);
        }
        pm_id.get_ref().parent (id());
      }

      // ***************************************************************** //

      void transition_type::clear_connections ()
      {
        _connections.clear();
      }

      void transition_type::clear_place_map ()
      {
        _place_map.clear();
      }

      void transition_type::connection_place
        (const id::ref::connect& connection, const std::string& place)
      {
        if (connection.get().place() == place)
        {
          return;
        }

        if ( _connections.has
             ( boost::make_tuple ( place
                                 , connection.get().port()
                                 , petri_net::edge::is_PT
                                   (connection.get().direction())
                                 )
             )
           )
        {
          throw std::runtime_error ( "tried reconnecting connection to place, "
                                     "but connection between between that place "
                                     "and port already exists in that direction"
                                   );
        }

        _connections.erase (connection);
        connection.get_ref().place_impl (place);
        _connections.push (connection);
      }

      void transition_type::connection_direction
        (const id::ref::connect& connection, const petri_net::edge::type& dir)
      {
        if (connection.get().direction() == dir)
        {
          return;
        }

        if ( ( petri_net::edge::is_PT (connection.get().direction())
             != petri_net::edge::is_PT (dir)
             )
           && _connections.has
             ( boost::make_tuple ( connection.get().place()
                                 , connection.get().port()
                                 , petri_net::edge::is_PT (dir)
                                 )
             )
           )
        {
          throw std::runtime_error ( "tried setting direction of connection, "
                                     "but connection between between that place "
                                     "and port already exists in that direction"
                                   );
        }

        _connections.erase (connection);
        connection.get_ref().direction_impl (dir);
        _connections.push (connection);
      }

      void transition_type::place_map_real
        (const id::ref::place_map& id, const std::string& real)
      {
        if (id.get().place_real() == real)
        {
          return;
        }

        if (_place_map.has (std::make_pair (id.get().place_virtual(), real)))
        {
          throw std::runtime_error ( "tried setting place_map's virtual place, "
                                     "but already having such a mapping."
                                   );
        }

        _place_map.erase (id);
        id.get_ref().place_real_impl (real);
        _place_map.push (id);
      }

      // ***************************************************************** //

      const conditions_type& transition_type::conditions() const
      {
        return _conditions;
      }
      void transition_type::add_conditions (const std::list<std::string>& other)
      {
        _conditions.insert (_conditions.end(), other.begin(), other.end());
      }

      // ***************************************************************** //

      namespace
      {
        class transition_specialize : public boost::static_visitor<void>
        {
        private:
          const type::type_map_type & map;
          const type::type_get_type & get;
          const xml::parse::structure_type_util::set_type & known_structs;
          state::type & state;

        public:
          transition_specialize
            ( const type::type_map_type & _map
            , const type::type_get_type & _get
            , const xml::parse::structure_type_util::set_type & _known_structs
            , state::type & _state
            )
              : map (_map)
              , get (_get)
              , known_structs (_known_structs)
              , state (_state)
          {}

          void operator () (const id::ref::use&) const { return; }
          void operator () (const id::ref::function& id_function) const
          {
            id_function.get_ref().specialize (map, get, known_structs, state);
          }
        };
      }

      void transition_type::specialize ( const type::type_map_type & map
                                       , const type::type_get_type & get
                                       , const xml::parse::structure_type_util::set_type & known_structs
                                       , state::type & state
                                       )
      {
        boost::apply_visitor
          ( transition_specialize ( map
                                  , get
                                  , known_structs
                                  , state
                                  )
          , function_or_use()
          );
      }

      // ***************************************************************** //

      namespace
      {
        class transition_sanity_check : public boost::static_visitor<void>
        {
        private:
          const state::type & state;

        public:
          transition_sanity_check (const state::type & _state)
            : state (_state)
          { }

          void operator () (const id::ref::use&) const { return; }
          void operator () (const id::ref::function& id_function) const
          {
            id_function.get().sanity_check (state);
          }
        };
      }

      void transition_type::sanity_check (const state::type & state) const
      {
        boost::apply_visitor ( transition_sanity_check (state)
                             , function_or_use()
                             );
      }

      // ***************************************************************** //

      //! \todo move to connect_type
      void transition_type::type_check ( const connect_type & connect
                                       , const state::type & state
                                       ) const
      {
        assert (has_parent());

        const std::string direction
          (petri_net::edge::enum_to_string (connect.direction()));

        const boost::optional<const id::ref::place&> id_place
          (connect.resolved_place());

        if (not id_place)
        {
          throw error::connect_to_nonexistent_place
            (make_reference_id(), connect.make_reference_id());
        }

        const boost::optional<const id::ref::port&> id_port
          (connect.resolved_port());

        if (not id_port)
        {
          throw error::connect_to_nonexistent_port
            (make_reference_id(), connect.make_reference_id());
        }

        if (id_place->get().signature() != id_port->get().signature())
        {
          throw error::connect_type_error ( make_reference_id()
                                          , connect.make_reference_id()
                                          , *id_port
                                          , *id_place
                                          );
        }
      }

      namespace
      {
        class transition_type_check : public boost::static_visitor<void>
        {
        private:
          const state::type & state;

        public:
          transition_type_check (const state::type & _state)
            : state (_state)
          { }

          void operator () (const id::ref::use&) const { return; }
          void operator () (const id::ref::function & id_function) const
          {
            id_function.get().type_check (state);
          }
        };
      }

      void transition_type::type_check (const state::type & state) const
      {
        BOOST_FOREACH (const connect_type& connect, connections().values())
        {
          type_check (connect, state);
        }

        boost::apply_visitor (transition_type_check (state), function_or_use());
      }

      const we::type::property::type& transition_type::properties() const
      {
        return _properties;
      }
      we::type::property::type& transition_type::properties()
      {
        return _properties;
      }

      // ******************************************************************* //

      boost::optional<pnet::type::signature::signature_type>
      transition_type::signature (const std::string& type) const
      {
        if (has_parent())
        {
          return parent()->signature (type);
        }
        return boost::none;
      }

      // ******************************************************************* //

      const transition_type::unique_key_type&
        transition_type::unique_key() const
      {
        return name();
      }

      // ******************************************************************* //

      namespace
      {
        typedef transition_type::function_or_use_type function_or_use_type;

        class visitor_clone
          : public boost::static_visitor<function_or_use_type>
        {
        public:
          visitor_clone ( const id::transition& new_id
                        , id::mapper* const mapper
                        )
            : _new_id (new_id)
            , _mapper (mapper)
          { }

          function_or_use_type operator() (const id::ref::function& id) const
          {
            return id.get().clone
              (function_type::make_parent (_new_id), _mapper);
          }
          function_or_use_type operator() (const id::ref::use& use) const
          {
            return use.get().clone (_new_id, _mapper);
          }

        private:
          const id::transition& _new_id;
          id::mapper* const _mapper;
        };
      }

      id::ref::transition transition_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return transition_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
          , _function_or_use
          ? boost::make_optional
            ( boost::apply_visitor ( visitor_clone (new_id, new_mapper)
                                   , *_function_or_use
                                   )
            )
          : boost::none
          , _name
          , _connections.clone (new_id, new_mapper)
          , _place_map.clone (new_id, new_mapper)
          , structs
          , _conditions
          , requirements
          , priority
          , finline
          , internal
          , _properties
          ).make_reference_id();
      }

      // ******************************************************************* //

      namespace
      {
        place_map_map_type::mapped_type
        get_pid (const place_map_map_type & pid_of_place, const std::string name)
        {
          const place_map_map_type::const_iterator pos (pid_of_place.find (name));

          if (pos == pid_of_place.end())
          {
            std::ostringstream s;
            s << __FILE__ << " [" << __LINE__ << "]: "
              << "missing place " << name << " in pid_of_place";
            throw error::strange (s.str());
          }

          return pos->second;
        }
      }

      void transition_synthesize
        ( const id::ref::transition & id_transition
        , const state::type & state
        , petri_net::net & we_net
        , const place_map_map_type & pids
        )
      {
        const transition_type& trans (id_transition.get());

        if (trans.connections().empty())
          {
            state.warn
              ( warning::independent_transition ( trans.name()
                                                , state.file_in_progress()
                                                )
              );
          }

        const id::ref::function id_function (trans.resolved_function());
        const function_type& fun (id_function.get());

        BOOST_FOREACH (const port_type& port_in, fun.ports().values())
        {
          if (port_in.direction() == we::type::PORT_IN)
          {
            const boost::optional<const id::ref::port&> id_port_out
              (fun.get_port_out (port_in.name()));

            if (  id_port_out
               && id_port_out->get().signature() != port_in.signature()
               )
            {
              state.warn
                ( warning::conflicting_port_types ( id_transition
                                                  , port_in.make_reference_id()
                                                  , *id_port_out
                                                  , state.file_in_progress()
                                                  )
                );
            }
          }
        }

        if (  fun.name()
           && (*fun.name() != trans.name())
           && (!rewrite::has_magic_prefix (trans.name()))
           )
        {
          state.warn ( warning::overwrite_function_name_trans
                       (id_transition, id_function)
                     );
        }

        if (fun.internal && trans.internal && *trans.internal != *fun.internal)
        {
          state.warn ( warning::overwrite_function_internal_trans
                       (id_transition, id_function)
                     );
        }

        if (  not trans.priority // WORK HERE: make it work with prio
           && (
               (  !state.synthesize_virtual_places()
               && !trans.place_map().empty()
               )
               || (  !state.no_inline()
                  && trans.finline.get_value_or(false)
                  && fun.is_net()
                  )
              )
           )
          { // unfold

            // set a prefix
            const std::string prefix (rewrite::mk_prefix (trans.name()));

            place_map_map_type place_map_map;

            BOOST_FOREACH ( const place_map_type& place_map
                          , trans.place_map().values()
                          )
              {
                const place_map_map_type::const_iterator pid
                  (pids.find (place_map.place_real()));

                if (pid == pids.end())
                  {
                    throw
                      error::real_place_missing ( place_map.place_virtual()
                                                , place_map.place_real()
                                                , trans.name()
                                                , state.file_in_progress()
                                                );
                  }

                place_map_map[prefix + place_map.place_virtual()] = pid->second;
              }

            net_type& net (fun.get_net()->get_ref());
            net.set_prefix (prefix);

            // synthesize into this level
            const place_map_map_type pid_of_place
              (net_synthesize (we_net, place_map_map, net, state));

            net.remove_prefix (prefix);

            // go in the subnet
            const std::string cond_in
              ((fun.conditions() + trans.conditions()).flatten());

            expr::parse::parser parsed_condition_in
              ( util::we_parse ( cond_in
                               , "condition"
                               , "unfold"
                               , trans.name()
                               , trans.position_of_definition().path()
                               )
              );

            we::type::property::type properties (fun.properties());
            util::property::join (state, properties, trans.properties());

            //! \todo It seems like this should be getting the
            //! requirements of the inlined transition. Or all
            //! inlined transitions?
            we::type::transition_t trans_in
              ( prefix + "IN"
              , we::type::expression_t()
              , condition::type (cond_in, parsed_condition_in)
              , true
              , properties
              );

            BOOST_FOREACH (const port_type& port, fun.ports().values())
            {
              if (port.direction() == we::type::PORT_IN)
              {
                trans_in.add_port
                  ( we::type::port_t ( port.name()
                                     , we::type::PORT_IN
                                     , port.signature_or_throw()
                                     , port.properties()
                                     )
                  );
                trans_in.add_port
                  ( we::type::port_t ( port.name()
                                     , we::type::PORT_OUT
                                     , port.signature_or_throw()
                                     , port.properties()
                                     )
                  );

                if (port.place)
                {
                  trans_in.add_connection
                    ( port.name()
                    , get_pid (pid_of_place , prefix + *port.place)
                    , port.properties()
                    );
                }
              }
            }

            BOOST_FOREACH ( const connect_type& connect
                          , trans.connections().values()
                          )
            {
              if (petri_net::edge::is_PT (connect.direction()))
              {
                trans_in.add_connection ( get_pid (pids, connect.place())
                                        , connect.port()
                                        , connect.properties()
                                        );
              }
            }

            const petri_net::transition_id_type tid_in
              (we_net.add_transition (trans_in));

            BOOST_FOREACH (const port_type& port, fun.ports().values())
            {
              if (port.direction() == we::type::PORT_IN && port.place)
              {
                we_net.add_connection
                  ( petri_net::connection_t ( petri_net::edge::TP
                                            , tid_in
                                            , get_pid ( pid_of_place
                                                      , prefix + *port.place
                                                      )
                                            )
                  );
              }
            }

            BOOST_FOREACH ( const connect_type& connect
                          , trans.connections().values()
                          )
            {
              if (petri_net::edge::is_PT (connect.direction()))
              {
                we_net.add_connection
                  ( petri_net::connection_t ( connect.direction()
                                            , tid_in
                                            , get_pid (pids, connect.place())
                                            )
                  );
              }
            }

            // going out of the subnet
            const std::string cond_out ("true");

            expr::parse::parser parsed_condition_out
              ( util::we_parse ( cond_out
                               , "condition"
                               , "unfold"
                               , trans.name()
                               , trans.position_of_definition().path()
                               )
              );

            we::type::transition_t trans_out
              ( prefix + "OUT"
              , we::type::expression_t()
              , condition::type (cond_out, parsed_condition_out)
              , true
              , properties
              );

            BOOST_FOREACH (const port_type& port, fun.ports().values())
            {
              if (port.direction() == we::type::PORT_OUT)
              {
                trans_out.add_port
                  ( we::type::port_t ( port.name()
                                     , we::type::PORT_IN
                                     , port.signature_or_throw()
                                     , port.properties()
                                     )
                  );
                trans_out.add_port
                  ( we::type::port_t ( port.name()
                                     , we::type::PORT_OUT
                                     , port.signature_or_throw()
                                     , port.properties()
                                     )
                  );

                if (port.place)
                {
                  trans_out.add_connection
                    ( get_pid (pid_of_place , prefix + *port.place)
                    , port.name()
                    , port.properties()
                    );
                }
              }
            }

            std::size_t num_outport (0);

            BOOST_FOREACH ( const connect_type& connect
                          , trans.connections().values()
                          )
            {
              if (!petri_net::edge::is_PT (connect.direction()))
              {
                trans_out.add_connection
                  ( connect.port()
                  , get_pid (pids, connect.place())
                  , connect.properties()
                  );

                ++num_outport;
              }
            }

            if (num_outport > 1)
              {
                const std::string
                  key ("pnetc.warning.inline-many-output-ports");
                const boost::optional<const ::we::type::property::value_type&>
                  warning_switch (properties.get_maybe_val (key));

                if (!warning_switch || *warning_switch != "off")
                  {
                    state.warn ( warning::inline_many_output_ports
                                 ( trans.name()
                                 , state.file_in_progress()
                                 )
                               );
                  }
              }

            const petri_net::transition_id_type tid_out
              (we_net.add_transition (trans_out));

            BOOST_FOREACH (const port_type& port, fun.ports().values())
            {
              if (port.direction() == we::type::PORT_OUT && port.place)
              {
                we_net.add_connection
                  ( petri_net::connection_t ( petri_net::edge::PT
                                            , tid_out
                                            , get_pid ( pid_of_place
                                                      , prefix + *port.place
                                                      )
                                            )
                  );
              }
            }

            BOOST_FOREACH ( const connect_type& connect
                          , trans.connections().values()
                          )
            {
              if (!petri_net::edge::is_PT (connect.direction()))
              {
                we_net.add_connection
                  ( petri_net::connection_t ( connect.direction()
                                            , tid_out
                                            , get_pid (pids, connect.place())
                                            )
                  );
              }
            }
          } // unfold

        else

          { // not unfold

            // set the real-property
            BOOST_FOREACH ( const place_map_type& place_map
                          , trans.place_map().values()
                          )
              {
                const place_map_map_type::const_iterator pid
                  (pids.find (place_map.place_real()));

                if (pid == pids.end())
                  {
                    throw
                      error::real_place_missing ( place_map.place_virtual()
                                                , place_map.place_real()
                                                , trans.name()
                                                , state.file_in_progress()
                                                );
                  }

                //! \todo eliminate the hack that stores the real
                //! place in the properties
                place::type we_place (we_net.get_place (pid->second));

                std::ostringstream path;

                path << "real" << "." << trans.name();

                we_place.property().set ( path.str()
                                        , place_map.place_virtual()
                                        );

                we_net.modify_place (pid->second, we_place);
              }

            we::type::transition_t we_trans
              ( fun.synthesize ( trans.name()
                               , state
                               , trans.internal
                               , trans.conditions()
                               , trans.properties()
                               , trans.requirements
                               )
              );

            BOOST_FOREACH ( const connect_type& connect
                          , trans.connections().values()
                          )
            {
              if (petri_net::edge::is_PT (connect.direction()))
              {
                we_trans.add_connection
                  ( get_pid (pids, connect.place())
                  , connect.port()
                  , connect.properties()
                  );
              }
              else
              {
                we_trans.add_connection
                  ( connect.port()
                  , get_pid (pids, connect.place())
                  , connect.properties()
                  );
              }
            }

            const petri_net::transition_id_type tid
              (we_net.add_transition (we_trans));

            if (trans.priority)
              {
                we_net.set_transition_priority (tid, *trans.priority);
              }

            BOOST_FOREACH ( const connect_type& connect
                          , trans.connections().values()
                          )
            {
              we_net.add_connection
                ( petri_net::connection_t ( connect.direction()
                                          , tid
                                          , get_pid (pids, connect.place())
                                          )
                );
            }
          } // not unfold

        return;
      }

      // ******************************************************************* //

      namespace dump
      {
        namespace
        {
          class dump_visitor : public boost::static_visitor<void>
          {
          private:
            ::fhg::util::xml::xmlstream & s;

          public:
            dump_visitor ( ::fhg::util::xml::xmlstream & _s)
              : s (_s)
            {}

            void operator () (const id::ref::use& use) const
            {
              ::xml::parse::type::dump::dump (s, use.get());
            }
            void operator () (const id::ref::function& id_function) const
            {
              ::xml::parse::type::dump::dump (s, id_function.get());
            }
          };
        }

        void dump ( ::fhg::util::xml::xmlstream & s
                  , const transition_type & t
                  )
        {
          s.open ("transition");
          s.attr ("name", t.name());
          s.attr ("priority", t.priority);
          s.attr ("inline", t.finline);
          s.attr ("internal", t.internal);

          ::we::type::property::dump::dump (s, t.properties());
          ::xml::parse::type::dump::dump (s, t.requirements);

          boost::apply_visitor (dump_visitor (s), t.function_or_use());

          dumps (s, t.place_map().values());
          dumps (s, t.connections().values());

          BOOST_FOREACH (const std::string& cond, t.conditions())
          {
            s.open ("condition");
            s.content (cond);
            s.close();
          }

          s.close ();
        }
      }
    }
  }
}
