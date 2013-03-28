// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/function.hpp>

#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/use.hpp>
#include <xml/parse/type/link.hpp>
#include <xml/parse/util/property.hpp>
#include <xml/parse/util/weparse.hpp>

#include <xml/parse/type/dumps.hpp>

#include <fhg/util/boost/variant.hpp>
#include <fhg/util/cpp.hpp>

#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/net.fwd.hpp>

#include <we/type/literal/cpp.hpp>
#include <we/type/signature/cpp.hpp>

#include <we/type/id.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      // ******************************************************************* //

      namespace
      {
        class visitor_reparent : public boost::static_visitor<void>
        {
        public:
          visitor_reparent (const id::function& parent)
            : _parent (parent)
          { }

          void operator() (const id::ref::expression& id) const
          {
            id.get_ref().parent (_parent);
          }
          void operator() (const id::ref::module& id) const
          {
            id.get_ref().parent (_parent);
          }
          void operator() (const id::ref::net& id) const
          {
            id.get_ref().parent (_parent);
          }

        private:
          const id::function& _parent;
        };

        const function_type::content_type& reparent
          ( const function_type::content_type& content
          , const id::function& parent
          )
        {
          boost::apply_visitor (visitor_reparent (parent), content);
          return content;
        }
      }

      function_type::function_type ( ID_CONS_PARAM(function)
                                   , const boost::optional<parent_id_type>& parent
                                   , const util::position_type& pod
                                   , const content_type& content
                                   )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , _parent (parent)
        , contains_a_module_call (false)
        , _content (reparent (content, _id))
      {
        _id_mapper->put (_id, *this);
      }

      function_type::function_type ( ID_CONS_PARAM(function)
                                   , const boost::optional<parent_id_type>& parent
                                   , const util::position_type& pod
                                   , const boost::optional<std::string>& name
                                   , const boost::optional<bool>& internal
                                   , const content_type& content
                                   )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , _parent (parent)
        , _name (name)
        , contains_a_module_call (false)
        , internal (internal)
        , _content (reparent (content, _id))
      {
        _id_mapper->put (_id, *this);
      }

      function_type::function_type
        ( ID_CONS_PARAM(function)
        , const boost::optional<parent_id_type>& parent
        , const util::position_type& pod
        , const boost::optional<std::string>& name
        , const ports_type& ports
        , const typenames_type& typenames
        , const bool& contains_a_module_call
        , const boost::optional<bool>& internal
        , const structs_type& structs
        , const conditions_type& conditions
        , const requirements_type& requirements
        , const content_type& content
        , const we::type::property::type& properties
        )
        : with_position_of_definition (pod)
        , ID_INITIALIZE()
        , _parent (parent)
        , _name (name)
        , _ports (ports, _id)
        , _typenames (typenames)
        , contains_a_module_call (contains_a_module_call)
        , internal (internal)
        , structs (structs)
        , _conditions (conditions)
        , requirements (requirements)
        , _content (reparent (content, _id))
        , _properties (properties)
      {
        _id_mapper->put (_id, *this);
      }

      const function_type::content_type& function_type::content() const
      {
        return _content;
      }
      function_type::content_type& function_type::content()
      {
        return _content;
      }
      const function_type::content_type&
        function_type::content (const content_type& content_)
      {
        return _content = reparent (content_, id());
      }

      // ******************************************************************* //

      bool function_type::is_net() const
      {
        return fhg::util::boost::is_of_type<id::ref::net> (content());
      }

      boost::optional<const id::ref::net&> function_type::get_net() const
      {
        return fhg::util::boost::get_or_none<id::ref::net> (content());
      }

      // ******************************************************************* //

      const boost::optional<std::string>& function_type::name() const
      {
        return _name;
      }
      const boost::optional<std::string>&
        function_type::name_impl (const boost::optional<std::string>& name)
      {
        return _name = name;
      }
      const boost::optional<std::string>&
        function_type::name (const boost::optional<std::string>& name)
      {
        if (parent_net())
        {
          if (!name)
          {
            throw std::runtime_error ( "Tried setting function to anonymous "
                                       "while being in a net. This should never "
                                       "happen as function's unique key is name."
                                     );
          }

          parent_net()->get_ref().rename (make_reference_id(), *name);
          return _name;
        }
        return name_impl (name);
      }

      const boost::optional<function_type::parent_id_type>& function_type::parent() const
      {
        return _parent;
      }

      bool function_type::has_parent() const
      {
        return _parent;
      }

      void function_type::unparent()
      {
        _parent = boost::none;
      }
      void function_type::parent (const parent_id_type& parent)
      {
        _parent = boost::make_optional (parent);
      }

      namespace
      {
        template<typename id_type, typename id_ref_type>
        class visitor_get_parent
          : public boost::static_visitor<boost::optional<id_ref_type> >
        {
        public:
          visitor_get_parent (id::mapper* id_mapper)
            : _id_mapper (id_mapper)
          { }

          boost::optional<id_ref_type> operator() (const id_type& id) const
          {
            return _id_mapper->get (id)->make_reference_id();
          }

          template<typename other_types>
            boost::optional<id_ref_type> operator() (const other_types&) const
          {
            return boost::none;
          }

        private:
          id::mapper* _id_mapper;
        };
      }

      boost::optional<id::ref::transition>
        function_type::parent_transition() const
      {
        if (!_parent)
        {
          return boost::none;
        }

        return boost::apply_visitor
          ( visitor_get_parent<id::transition, id::ref::transition> (id_mapper())
          , *_parent
          );
      }

      boost::optional<id::ref::tmpl>
        function_type::parent_tmpl() const
      {
        if (!_parent)
        {
          return boost::none;
        }

        return boost::apply_visitor
          ( visitor_get_parent<id::tmpl, id::ref::tmpl> (id_mapper())
          , *_parent
          );
      }

      boost::optional<id::ref::net>
        function_type::parent_net() const
      {
        if (!_parent)
        {
          return boost::none;
        }

        return boost::apply_visitor
          ( visitor_get_parent<id::net, id::ref::net> (id_mapper())
          , *_parent
          );
      }

      namespace
      {
        class visitor_get_function
          : public boost::static_visitor<boost::optional<const id::ref::function&> >
        {
        private:
          const std::string& _name;
          const id::mapper* _id_mapper;

        public:
          visitor_get_function ( const std::string& name
                               , const id::mapper* id_mapper
                               )
            : _name (name)
            , _id_mapper (id_mapper)
          {}

          template<typename ID>
          boost::optional<const id::ref::function&> operator () (ID id) const
          {
            return _id_mapper->get(id)->get_function (_name);
          }
        };
      }

      boost::optional<const id::ref::function&>
      function_type::get_function (const std::string& name) const
      {
        if (has_parent())
          {
            return boost::apply_visitor
              (visitor_get_function (name, id_mapper()), *parent());
          }

        return boost::none;
      }

      // ***************************************************************** //

      void function_type::push_port (const id::ref::port& id)
      {
        const id::ref::port& id_old (_ports.push (id));

        if (id_old != id)
        {
          throw error::duplicate_port (id_old, id);
        }

        id.get_ref().parent (_id);

        if (id.get().direction() != we::type::PORT_TUNNEL)
        {
          boost::optional<const id::ref::port&> id_other
            ( _ports.get ( std::make_pair ( id.get().name()
                                          , id.get().direction()
                                          == we::type::PORT_IN
                                          ? we::type::PORT_OUT
                                          : we::type::PORT_IN
                                          )
                         )
            );

          if (id_other && id.get().signature() != id_other->get().signature())
          {
            throw error::port_type_mismatch (id, *id_other);
          }
        }
      }

      void function_type::remove_port (const id::ref::port& id)
      {
        _ports.erase (id);
      }

      const function_type::ports_type& function_type::ports() const
      {
        return _ports;
      }

      boost::optional<const id::ref::port&>
      function_type::get_port_in (const std::string & name) const
      {
        return ports().get (std::make_pair (name, we::type::PORT_IN));
      }

      boost::optional<const id::ref::port&>
      function_type::get_port_out (const std::string & name) const
      {
        return ports().get (std::make_pair (name, we::type::PORT_OUT));
      }

      bool function_type::is_known_port_in (const std::string & name) const
      {
        return get_port_in (name);
      }

      bool function_type::is_known_port_out (const std::string & name) const
      {
        return get_port_out (name);
      }

      bool function_type::is_known_port (const std::string & name) const
      {
        return is_known_port_in (name) || is_known_port_out (name);
      }

      bool function_type::is_known_port_inout (const std::string & name) const
      {
        return is_known_port_in (name) && is_known_port_out (name);
      }

      bool function_type::is_known_tunnel (const std::string& name) const
      {
        return ports().has (std::make_pair (name, we::type::PORT_TUNNEL));
      }

      void function_type::rename (const id::ref::port& id, const std::string& n)
      {
        if (id.get().name() == n)
        {
          return;
        }

        if (_ports.has (std::make_pair (n, id.get().direction())))
        {
          throw std::runtime_error
            ("tried renaming port, but port with given name exists");
        }

        _ports.erase (id);
        id.get_ref().name_impl (n);
        _ports.push (id);
      }

      void function_type::port_direction
          (const id::ref::port& id, const we::type::PortDirection& direction)
      {
        if (id.get().direction() == direction)
        {
          return;
        }

        if (_ports.has (std::make_pair (id.get().name(), direction)))
        {
          throw std::runtime_error
            ("tried changing port direction, but port with combination exists");
        }

        _ports.erase (id);
        id.get_ref().direction_impl (direction);
        _ports.push (id);
      }

      // ***************************************************************** //

      const function_type::typenames_type& function_type::typenames () const
      {
        return _typenames;
      }
      void function_type::insert_typename (const std::string& tn)
      {
        _typenames.insert (tn);
      }

      // ***************************************************************** //

      namespace
      {
        class visitor_append_expressions
          : public boost::static_visitor<void>
        {
        private:
          const expressions_type& _expressions;

        public:
          visitor_append_expressions (const expressions_type& expressions)
            : _expressions (expressions)
          { }

          void operator () (id::ref::expression & id_expression) const
          {
            id_expression.get_ref().append (_expressions);
          }

          template<typename T>
          void operator () (T &) const
          {
            throw std::runtime_error ("BUMMER: join for non expression!");
          }
        };
      }

      void function_type::add_expression (const expressions_type & es)
      {
        boost::apply_visitor (visitor_append_expressions (es), content());
      }

      // ***************************************************************** //

      const conditions_type& function_type::conditions() const
      {
        return _conditions;
      }
      void function_type::add_conditions (const std::list<std::string>& other)
      {
        _conditions.insert (_conditions.end(), other.begin(), other.end());
      }

      std::string conditions_type::flatten() const
      {
        return empty()
          ? "true"
          : fhg::util::join (begin(), end(), " && ", "(", ")");
      }

      conditions_type operator+ (conditions_type lhs, const conditions_type& rhs)
      {
        lhs.insert (lhs.end(), rhs.begin(), rhs.end());
        return lhs;
      }

      // ***************************************************************** //

      xml::parse::structure_type::forbidden_type
        function_type::forbidden_below (void) const
      {
        xml::parse::structure_type::forbidden_type forbidden;

        BOOST_FOREACH (const port_type& port, ports().values())
        {
          forbidden.insert (std::make_pair (port.type(), port.name()));
        }

        return forbidden;
      }

      // ***************************************************************** //

      boost::optional<signature::type>
      function_type::signature (const std::string& type) const
      {
        const structs_type::const_iterator pos
          ( std::find_if ( structs.begin()
                         , structs.end()
                         , boost::bind ( parse::structure_type::struct_by_name
                                       , type
                                       , _1
                                       )
                         )
          );

        if (pos != structs.end())
        {
          return signature::type
            ( parse::structure_type::resolve_with_fun
              (*pos, boost::bind (&function_type::signature, *this, _1))
            , pos->name()
            );
        }

        if (parent_transition())
        {
          return parent_transition()->get().signature (type);
        }
        else if (parent_tmpl())
        {
          return parent_tmpl()->get().signature (type);
        }
        else if (parent_net())
        {
          return parent_net()->get().signature (type);
        }

        return boost::none;
      }

      // ***************************************************************** //

      namespace
      {
        class function_sanity_check : public boost::static_visitor<void>
        {
        private:
          const state::type & state;

        public:
          function_sanity_check (const state::type & _state)
            : state (_state)
          {}

          void operator () (const id::ref::expression &) const { return; }
          void operator () (const id::ref::module& id) const
          {
            id.get().sanity_check();
          }
          void operator () (const id::ref::net& id) const
          {
            id.get().sanity_check (state);
          }
        };
      }

      void function_type::sanity_check (const state::type & state) const
      {
        boost::apply_visitor (function_sanity_check (state), content());
      }

      // ***************************************************************** //

      namespace
      {
        class function_type_check : public boost::static_visitor<void>
        {
        public:
          function_type_check (const state::type& state)
            : _state (state)
          { }

          void operator() (const id::ref::expression&) const
          {
          }
          void operator() (const id::ref::module&) const
          {
          }
          void operator() (const id::ref::net& id) const
          {
            id.get().type_check (_state);
          }

        private:
          const state::type& _state;
        };
      }

      void function_type::type_check (const state::type & state) const
      {
        BOOST_FOREACH (const port_type& port, ports().values())
        {
          port.type_check (position_of_definition().path(), state);
        }

        boost::apply_visitor (function_type_check (state), content());
      }

      // ***************************************************************** //

      class function_synthesize
        : public boost::static_visitor<we::type::transition_t>
      {
      private:
        const std::string& _name;
        const state::type & state;
        const function_type& fun;
        const boost::optional<bool>& _internal;
        const conditions_type& _conditions;
        we::type::property::type _properties;
        const requirements_type& _trans_requirements;

        typedef we::type::transition_t we_transition_type;

        typedef petri_net::net we_net_type;
        typedef we::type::module_call_t we_module_type;
        typedef we::type::expression_t we_expr_type;
        typedef we_transition_type::preparsed_cond_type we_cond_type;

        typedef we_transition_type::requirement_t we_requirement_type;

        typedef boost::unordered_map< std::string
                                    , petri_net::place_id_type
                                    > pid_of_place_type;

        void add_ports ( we_transition_type & trans
                       , const function_type::ports_type& ports
                       ) const
        {
          BOOST_FOREACH (const port_type& port, ports.values())
            {
              trans.add_port ( port.name()
                             , port.signature_or_throw()
                             , port.direction()
                             , port.properties()
                             );
            }
        }

        template<typename Map>
        typename Map::mapped_type
        get_pid (const Map & pid_of_place, const std::string name) const
        {
          const typename Map::const_iterator pos (pid_of_place.find (name));

          return pos->second;
        }

        template<typename Map>
        void add_ports ( we_transition_type & trans
                       , const function_type::ports_type& ports
                       , const Map & pid_of_place
                       ) const
        {
          BOOST_FOREACH (const port_type& port, ports.values())
            {
              if (not port.place)
                {
                  trans.add_port ( port.name()
                                 , port.signature_or_throw()
                                 , port.direction()
                                 , port.properties()
                                 );
                }
              else
                {
                  // basically safe, since type checking has verified
                  // the existence and type safety of the place to
                  // connect to

                  trans.add_port ( port.name()
                                 , port.signature_or_throw()
                                 , port.direction()
                                 , get_pid (pid_of_place, *port.place)
                                 , port.properties()
                                 )
                    ;
                }
            }
        }

        void add_requirements (we_transition_type& trans) const
        {
          requirements_type requirements (fun.requirements);
          requirements.join (_trans_requirements);

          for ( requirements_type::const_iterator r (requirements.begin())
              ; r != requirements.end()
              ; ++r
              )
          {
            trans.add_requirement (we_requirement_type (r->first, r->second));
          }
        }

        const std::string& name() const
        {
          return _name;
        }

        we_cond_type condition (void) const
        {
          const std::string cond ((fun.conditions() + _conditions).flatten());

          expr::parse::parser parsed_condition
            (util::we_parse (cond, "condition", "function", name(), fun.position_of_definition().path()));

          return we_cond_type (cond, parsed_condition);
        }

      public:
        function_synthesize ( const std::string& name
                            , const state::type& _state
                            , const function_type& _fun
                            , const boost::optional<bool>& internal
                            , const conditions_type& conditions
                            , const we::type::property::type& trans_properties
                            , const requirements_type& trans_requirements
                            )
          : _name (name)
          , state (_state)
          , fun (_fun)
          , _internal (internal)
          , _conditions (conditions)
          , _properties (trans_properties)
          , _trans_requirements (trans_requirements)
        {
          util::property::join (state, _properties, fun.properties());
        }

        we_transition_type
        operator () (const id::ref::expression& id_expression) const
        {
          const std::string expr (id_expression.get().expression());
          const expr::parse::parser parsed_expression
            (util::we_parse (expr, "expression", "function", name(), fun.position_of_definition().path()));

          we_transition_type trans
            ( name()
            , we_expr_type (expr, parsed_expression)
            , condition()
            , _internal.get_value_or (true)
            , _properties
            );

          add_ports (trans, fun.ports());
          add_requirements (trans);

          return trans;
        }

        we_transition_type operator () (const id::ref::module& id_mod) const
        {
          const module_type& mod (id_mod.get());

          we_transition_type trans
            ( name()
            , we_module_type (mod.name(), mod.function())
            , condition()
            , _internal.get_value_or (false)
            , _properties
            );

          add_ports (trans, fun.ports());
          add_requirements (trans);

          return trans;
        }

        we_transition_type operator () (const id::ref::net & id_net) const
        {
          we_net_type we_net;

          pid_of_place_type pid_of_place
            ( net_synthesize ( we_net
                             , place_map_map_type()
                             , id_net.get()
                             , state
                             )
            );

          we::type::property::type properties (_properties);

          util::property::join ( state
                               , properties
                               , id_net.get().properties()
                               );

          we_transition_type trans
            ( name()
            , we_net
            , condition()
            , _internal.get_value_or (true)
            , properties
            );

          add_ports (trans, fun.ports(), pid_of_place);
          add_requirements (trans);

          return trans;
        }
      };

      we::type::transition_t function_type::synthesize
        ( const std::string& name
        , const state::type& state
        , const boost::optional<bool>& trans_internal
        , const conditions_type& conditions
        , const we::type::property::type& trans_properties
        , const requirements_type& trans_requirements
        ) const
      {
        return boost::apply_visitor
          ( function_synthesize ( name
                                , state
                                , *this
                                , trans_internal ? trans_internal : internal
                                , conditions
                                , trans_properties
                                , trans_requirements
                                )
          , content()
          );
      }

      // ***************************************************************** //

      void function_type::specialize (state::type & state)
      {
        const type_map_type type_map_empty;
        const type_get_type type_get_empty;
        const xml::parse::structure_type::set_type known_empty;

        specialize ( type_map_empty
                   , type_get_empty
                   , known_empty
                   , state
                   );
      }

      namespace
      {
        class function_specialize : public boost::static_visitor<void>
        {
        private:
          const type::type_map_type & map;
          const type::type_get_type & get;
          const xml::parse::structure_type::set_type & known_structs;
          state::type & state;
          function_type & fun;

        public:
          function_specialize
            ( const type::type_map_type & _map
            , const type::type_get_type & _get
            , const xml::parse::structure_type::set_type & _known_structs
            , state::type & _state
            , function_type & _fun
            )
              : map (_map)
              , get (_get)
              , known_structs (_known_structs)
              , state (_state)
              , fun (_fun)
          {}

          void operator () (id::ref::expression &) const { return; }
          void operator () (id::ref::module &) const { return; }
          void operator () (id::ref::net & id) const
          {
            id.get_ref().specialize (map, get, known_structs, state);

            split_structs ( known_structs
                          , id.get_ref().structs
                          , fun.structs
                          , get
                          , state
                          );
          }
        };
      }

      void function_type::specialize ( const type_map_type & map
                                     , const type_get_type & get
                                     , const xml::parse::structure_type::set_type & known_structs
                                     , state::type & state
                                     )
      {
        BOOST_FOREACH (port_type& port, ports().values())
        {
          port.specialize (map, state);
        }

        specialize_structs (map, structs, state);

        namespace st = xml::parse::structure_type;

        boost::apply_visitor
          ( function_specialize
            ( map
            , get
            , st::join (known_structs, st::make (structs), state)
            , state
            , *this
            )
          , content()
          );
      }

      const we::type::property::type& function_type::properties() const
      {
        return _properties;
      }
      we::type::property::type& function_type::properties()
      {
        return _properties;
      }

      const function_type::unique_key_type& function_type::unique_key() const
      {
        //! \note anonymous functions can't be stored in unqiue, thus
        //! just indirect.
        return *name();
      }

      namespace
      {
        class visitor_clone
          : public boost::static_visitor<function_type::content_type>
        {
        public:
          visitor_clone ( const id::function& new_id
                        , id::mapper* const mapper
                        )
            : _new_id (new_id)
            , _mapper (mapper)
          { }
          template<typename ID_TYPE>
            function_type::content_type operator() (const ID_TYPE& id) const
          {
            return id.get().clone (_new_id, _mapper);
          }

        private:
          const id::function& _new_id;
          id::mapper* const _mapper;
        };
      }

      id::ref::function function_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return function_type
          ( new_id
          , new_mapper
          , parent
          , _position_of_definition
          , _name
          , _ports.clone (new_id, new_mapper)
          , _typenames
          , contains_a_module_call
          , internal
          , structs
          , _conditions
          , requirements
          , boost::apply_visitor (visitor_clone (new_id, new_mapper), content())
          , _properties
          ).make_reference_id();
      }

      // ***************************************************************** //

      fun_info_type::fun_info_type ( const std::string & _name
                                   , const std::string & _code
                                   , const std::list<std::string>& _ldflags
                                   , const std::list<std::string>& _cxxflags
                                   , const module_type::links_type & _links
                                   , const boost::filesystem::path & _path
                                   )
        : name (_name)
        , code (_code)
        , ldflags (_ldflags)
        , cxxflags (_cxxflags)
        , links (_links)
        , path (_path)
      { }

      bool fun_info_type::operator== (const fun_info_type & other) const
      {
        return name == other.name;
      }

      std::size_t hash_value (const fun_info_type & fi)
      {
        boost::hash<std::string> hasher;
        return hasher (fi.name);
      }

      typedef boost::unordered_set<fun_info_type> fun_infos_type;

      typedef boost::unordered_map<std::string,fun_infos_type> fun_info_map;

      typedef boost::filesystem::path path_t;

      void mk_wrapper ( const state::type & state
                      , const fun_info_map & m
                      )
      {
        namespace cpp_util = ::fhg::util::cpp;

        const path_t prefix (state.path_to_cpp());

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            const path_t file ( prefix
                              / cpp_util::path::op()
                              / cpp_util::make::cpp (mod->first)
                              );

            util::check_no_change_fstream stream (state, file);

            stream << "// GPI-Space generated: DO NOT EDIT THIS FILE!"
                   << std::endl << std::endl
              ;

            cpp_util::include (stream, "we/loader/macros.hpp");

            const fun_infos_type & funs (mod->second);

            for ( fun_infos_type::const_iterator fun (funs.begin())
                ; fun != funs.end()
                ; ++fun
                )
              {
                stream << std::endl << fun->code;
              }

            stream << std::endl;
            stream << "WE_MOD_INITIALIZE_START (" << mod->first << ");" << std::endl;
            stream << "{" << std::endl;
            for ( fun_infos_type::const_iterator fun (funs.begin())
                ; fun != funs.end()
                ; ++fun
                )
              {
                stream << "  WE_REGISTER_FUN_AS ("
                       << cpp_util::access::make ("", "pnetc", "op", mod->first, fun->name)
                       << ",\"" << fun->name << "\""
                       << ");"
                       << std::endl;
              }
            stream << "}" << std::endl;
            stream << "WE_MOD_INITIALIZE_END (" << mod->first << ");" << std::endl;

            stream << std::endl;
            stream << "WE_MOD_FINALIZE_START (" << mod->first << ");" << std::endl;
            stream << "{" << std::endl;
            stream << "}" << std::endl;
            stream << "WE_MOD_FINALIZE_END (" << mod->first << ");" << std::endl;
          }
      }

      void mk_makefile ( const state::type & state
                       , const fun_info_map & m
                       )
      {
        namespace cpp_util = ::fhg::util::cpp;

        const path_t prefix (state.path_to_cpp());
        const path_t file (prefix / "Makefile");

        const std::string file_global_cxxflags ("Makefile.CXXFLAGS");
        const std::string file_global_ldflags ("Makefile.LDFLAGS");

        util::check_no_change_fstream stream (state, file);

        stream << "# GPI-Space generated: DO NOT EDIT THIS FILE!"  << std::endl;
        stream                                                     << std::endl;

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            stream << "MODULES += " << cpp_util::make::mod_so (mod->first)
                                                                   << std::endl;
          }

        stream                                                     << std::endl;
        stream << "CXXFLAGS += -fPIC"                              << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef BOOST_ROOT"                              << std::endl;
        stream << "  $(warning !!!)"                               << std::endl;
        stream << "  $(warning !!! BOOST_ROOT EMPTY, ASSUMING /usr)"
                                                                   << std::endl;
        stream << "  $(warning !!! THIS IS PROBABLY NOT WHAT YOU WANT!)"
                                                                   << std::endl;
        stream << "  $(warning !!!)"                               << std::endl;
        stream << "  $(warning !!! Try to set BOOST_ROOT in the environment!)"
                                                                   << std::endl;
        stream << "  $(warning !!!)"                               << std::endl;
        stream << "  BOOST_ROOT = /usr"                            << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef CXX"                                     << std::endl;
        stream << "  $(error Variable CXX is not defined)"         << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef SDPA_INCLUDE"                            << std::endl;
        stream << "  ifndef SDPA_HOME"                             << std::endl;
        stream << "    $(error Neither SDPA_INCLUDE nor SDPA_HOME are set)"
                                                                   << std::endl;
        stream << "  else"                                         << std::endl;
        stream << "    SDPA_INCLUDE = $(SDPA_HOME)/include"        << std::endl;
        stream << "  endif"                                        << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "CXXFLAGS += -I."                                << std::endl;
        stream << "CXXFLAGS += -isystem $(SDPA_INCLUDE)"           << std::endl;
        stream << "CXXFLAGS += -isystem $(BOOST_ROOT)/include"     << std::endl;
        stream                                                     << std::endl;
        stream << "LDFLAGS += -L$(BOOST_ROOT)/lib"                 << std::endl;
        stream << "LDFLAGS += -lboost_serialization"               << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef CP"                                      << std::endl;
        stream << "  CP = $(shell which cp 2>/dev/null)"           << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef MKDIR"                                   << std::endl;
        stream << "  MKDIR = $(shell which mkdir 2>/dev/null)"     << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef RM"                                      << std::endl;
        stream << "  RM = $(shell which rm 2>/dev/null) -f"        << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;

        {
          util::check_no_change_fstream cxx
            (state, prefix / file_global_cxxflags);

          BOOST_FOREACH (std::string const& flag, state.gen_cxxflags())
          {
            cxx << "CXXFLAGS += " << flag << std::endl;
          }
        }

        {
          util::check_no_change_fstream ld
            (state, prefix / file_global_ldflags);

          BOOST_FOREACH (std::string const& flag, state.gen_ldflags())
          {
            ld << "LDFLAGS += " << flag << std::endl;
          }
        }

        stream << "include " << file_global_cxxflags               << std::endl;
        stream << "include " << file_global_ldflags                << std::endl;

        stream                                                     << std::endl;
        stream << ".PHONY: default modules depend install"         << std::endl;
        stream                                                     << std::endl;
        stream << "default: depend $(MODULES)"                     << std::endl;
        stream << "modules: depend $(MODULES) objcleandep"         << std::endl;
        stream                                                     << std::endl;
        stream << "ifeq \"$(CP)\" \"\""                            << std::endl;
        stream                                                     << std::endl;
        stream << "%.cpp:"                                         << std::endl;
        stream << "\t$(error Missing file '$@'.)"                  << std::endl;
        stream                                                     << std::endl;
        stream << "else"                                           << std::endl;
        stream                                                     << std::endl;
        stream << "%.cpp: %.cpp_tmpl"                              << std::endl;
        stream << "\t$(warning !!!)"                               << std::endl;
        stream << "\t$(warning !!! COPY $*.cpp_tmpl TO $*.cpp)"    << std::endl;
        stream << "\t$(warning !!! THIS IS PROBABLY NOT WHAT YOU WANT!)"
                                                                   << std::endl;
        stream << "\t$(warning !!!)"                               << std::endl;
        stream << "\t$(CP) $^ $@"                                  << std::endl;
        stream                                                     << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            stream << "####"                                       << std::endl;
            stream << "#### module-functions " << mod->first       << std::endl;
            stream << "####"                                       << std::endl;

            const std::string objs ("OBJ_" + mod->first);
            const fun_infos_type & funs (mod->second);
            const std::string ldflags ("LDFLAGS_" + mod->first);
            const std::string file_module_ldflags ("Makefile." + ldflags);

            util::check_no_change_fstream ldflags_module
              (state, prefix / file_module_ldflags);

            for ( fun_infos_type::const_iterator fun (funs.begin())
                ; fun != funs.end()
                ; ++fun
                )
              {
                stream << "##"                                     << std::endl;
                stream << "## function " << fun->name              << std::endl;
                stream << "##"                                     << std::endl;

                const std::string cxxflags
                  ("CXXFLAGS_" + mod->first + "_" + fun->name);
                const std::string file_function_cxxflags
                  ("Makefile." + cxxflags);

                stream << objs << " += "
                       << cpp_util::make::obj (mod->first, fun->name)
                                                                   << std::endl;

                BOOST_FOREACH (module_type::links_type::value_type const& link, fun->links)
                  {
                    stream << objs << " += "
                           << boost::filesystem::absolute
                              ( link.link
                                ( boost::bind ( &state::type::link_prefix_by_key
                                              , boost::ref (state)
                                              , _1
                                              )
                                )
                              , fun->path.parent_path()
                              ).string()
                           << std::endl;
                  }

                BOOST_FOREACH (const std::string& flag, fun->ldflags)
                  {
                    ldflags_module << ldflags << " += " << flag << std::endl;
                  }

                {
                  util::check_no_change_fstream cxxflags_function
                    (state, prefix / file_function_cxxflags);

                  BOOST_FOREACH (const std::string& flag, fun->cxxflags)
                  {
                    cxxflags_function << cxxflags << " += " << flag << std::endl;
                  }
                }

                stream << "include " << file_function_cxxflags     << std::endl;

                stream << cpp_util::make::obj (mod->first, fun->name)
                       << ": "
                       << cpp_util::make::cpp (mod->first, fun->name)
                       << " "
                       << file_global_cxxflags
                       << " "
                       << file_function_cxxflags
                                                                   << std::endl;
                stream << "\t$(CXX) $(CXXFLAGS)" << " $(" << cxxflags << ")"
                       << " -c $< -o $@"                           << std::endl;
                stream << cpp_util::make::dep (mod->first, fun->name)
                       << ": "
                       << cpp_util::make::cpp (mod->first, fun->name)
                       << " "
                       << file_global_cxxflags
                       << " "
                       << file_function_cxxflags
                                                                   << std::endl;
                stream << "\t$(CXX) $(CXXFLAGS)" << " $(" << cxxflags << ")"
                       << " -MM -MP -MT '"
                       << cpp_util::make::dep (mod->first, fun->name)
                       << "' -MT '"
                       << cpp_util::make::obj (mod->first, fun->name)
                       << "' $< -MF $@"                            << std::endl;
                stream << "ifneq \"$(wildcard "
                       << cpp_util::make::dep (mod->first, fun->name)
                       << ")\" \"\""                        << std::endl;
                stream << "  include "
                       << cpp_util::make::dep (mod->first, fun->name)
                                                                   << std::endl;
                stream << "endif"                                  << std::endl;
                stream << "DEPENDS += "
                       << cpp_util::make::dep (mod->first, fun->name)
                                                                   << std::endl;
                stream                                             << std::endl;
              }

            stream << "####"                                       << std::endl;
            stream << "#### module " << mod->first                 << std::endl;
            stream << "####"                                       << std::endl;

            stream << objs << " += " << cpp_util::make::obj (mod->first)
                                                                   << std::endl;

            const std::string obj_cpp (( cpp_util::path::op()
                                       / cpp_util::make::cpp (mod->first)
                                       ).string()
                                      );

            stream << cpp_util::make::obj (mod->first)
                   << ": "
                   << obj_cpp
                   << " "
                   << file_global_cxxflags                         << std::endl;
            stream << "\t$(CXX) $(CXXFLAGS) -c $< -o $@"           << std::endl;
            stream << cpp_util::make::dep (mod->first)
                   << ": "
                   << obj_cpp
                   << " "
                   << file_global_cxxflags                         << std::endl;
            stream << "\t$(CXX) $(CXXFLAGS)"
                   << " -MM -MP -MT '"
                   << cpp_util::make::dep (mod->first)
                   << "' -MT '"
                   << cpp_util::make::obj (mod->first)
                   << "' $< -MF $@"                                << std::endl;
            stream << "ifneq \"$(wildcard "
                   << cpp_util::make::dep (mod->first)
                   << ")\" \"\""                            << std::endl;
            stream << "  include "
                   << cpp_util::make::dep (mod->first)             << std::endl;
            stream << "endif"                                      << std::endl;
            stream << "DEPENDS += "
                   << cpp_util::make::dep (mod->first)             << std::endl;

            stream                                                 << std::endl;
            stream << "include " << file_module_ldflags            << std::endl;
            stream                                                 << std::endl;
            stream << cpp_util::make::mod_so (mod->first)
                   << ": $(" << objs << ")"
                   << " "
                   << file_global_ldflags
                   << " "
                   << file_module_ldflags                          << std::endl;
            stream << "\t$(CXX)"
                   << " -shared $(" << objs << ") -o $@"
                   << " $(" << ldflags << ")"
                   << " $(LDFLAGS)"                                << std::endl;
            stream                                                 << std::endl;
          }

        stream << "####"                                           << std::endl;
        stream << "#### modules finished"                          << std::endl;
        stream << "####"                                           << std::endl;
        stream                                                     << std::endl;

        stream << "ifeq \"$(LIB_DESTDIR)\" \"\""                   << std::endl;
        stream                                                     << std::endl;
        stream << "install:"                                       << std::endl;
        stream << "\t$(error variable LIB_DESTDIR empty.)"         << std::endl;
        stream                                                     << std::endl;
        stream << "else"                                           << std::endl;
        stream                                                     << std::endl;
        stream << "ifeq \"$(CP)\" \"\""                            << std::endl;
        stream                                                     << std::endl;
        stream << "install:"                                       << std::endl;
        stream << "\t$(error variable CP empty.)"                  << std::endl;
        stream                                                     << std::endl;
        stream << "else"                                           << std::endl;
        stream                                                     << std::endl;
        stream << "ifeq \"$(MKDIR)\" \"\""                         << std::endl;
        stream                                                     << std::endl;
        stream << "$(LIB_DESTDIR):"                                << std::endl;
        stream << "\t$(error Could not create installation directory: Variable 'mkdir' empty.)"
                                                                   << std::endl;
        stream                                                     << std::endl;
        stream << "else"                                           << std::endl;
        stream                                                     << std::endl;
        stream << "$(LIB_DESTDIR):"                                << std::endl;
        stream << "\t@$(MKDIR) -v -p $(LIB_DESTDIR)"               << std::endl;
        stream                                                     << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            stream << cpp_util::make::mod_so_install (mod->first)
                   << ": " << cpp_util::make::mod_so (mod->first)
                   << " $(LIB_DESTDIR)"                            << std::endl;
            stream << "\t@$(CP) -v $< $@"                          << std::endl;
          }

        stream                                                     << std::endl;

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            stream << "MODULES_INSTALL += "
                   << cpp_util::make::mod_so_install (mod->first)  << std::endl;
          }

        stream                                                     << std::endl;
        stream << "install: $(MODULES_INSTALL)"                    << std::endl;

        stream                                                     << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;

        stream << "depend: $(DEPENDS)"                             << std::endl;
        stream                                                     << std::endl;
        stream << "ifeq \"$(RM)\" \"\""                            << std::endl;
        stream                                                     << std::endl;
        stream << "clean:"                                         << std::endl;
        stream << "depclean:"                                      << std::endl;
        stream << "objclean:"                                      << std::endl;
        stream << "modclean:"                                      << std::endl;
        stream << "objcleandep:"                                   << std::endl;
        stream << "\t$(error Variable RM empty.)"                  << std::endl;
        stream                                                     << std::endl;
        stream << "else"                                           << std::endl;
        stream                                                     << std::endl;
        stream << ".PHONY: clean depclean objclean modclean objcleandep"
                                                                   << std::endl;
        stream                                                     << std::endl;
        stream << "clean: objclean modclean depclean"              << std::endl;
        stream                                                     << std::endl;
        stream << "depclean:"                                      << std::endl;
        stream << "\t-$(RM) $(DEPENDS)"                            << std::endl;
        stream                                                     << std::endl;
        stream << "objclean:"                                      << std::endl;

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            stream << "\t-$(RM) $(OBJ_" << mod->first << ")"        << std::endl;
          }

        stream                                                     << std::endl;
        stream << "objcleandep: $(MODULES)"                        << std::endl;

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            stream << "\t-$(RM) $(OBJ_" << mod->first << ")"       << std::endl;
          }

        stream                                                     << std::endl;
        stream << "modclean:"                                      << std::endl;
        stream << "\t-$(RM) $(MODULES)"                            << std::endl;
        stream                                                     << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
      }

      typedef boost::unordered_map<std::string, module_type>
        mc_by_function_type;
      typedef boost::unordered_map<std::string, mc_by_function_type>
        mcs_type;

      bool find_module_calls ( const state::type &
                             , const id::ref::function &
                             , fun_info_map &
                             , mcs_type &
                             );

      namespace
      {
        class transition_find_module_calls_visitor
          : public boost::static_visitor<bool>
        {
        private:
          const state::type & state;
          const id::ref::net & _id_net;
          const transition_type & trans;
          fun_info_map & m;
          mcs_type& mcs;

        public:
          transition_find_module_calls_visitor ( const state::type & _state
                                               , const id::ref::net& id_net
                                               , const transition_type & _trans
                                               , fun_info_map & _m
                                               , mcs_type& _mcs
                                       )
            : state (_state)
            , _id_net (id_net)
            , trans (_trans)
            , m (_m)
            , mcs (_mcs)
          {}

          bool operator () (const id::ref::function & id_function) const
          {
            return find_module_calls (state, id_function, m, mcs);
          }

          bool operator () (const id::ref::use& u) const
          {
            boost::optional<const id::ref::function&> id_function
              (_id_net.get_ref().get_function (u.get().name()));

            if (not id_function)
              {
                throw error::unknown_function
                  (u.get().name(), trans.make_reference_id());
              }

            return find_module_calls (state, *id_function, m, mcs);
          }
        };
      }

      bool find_module_calls ( const state::type & state
                             , id::ref::net & id_net
                             , fun_info_map & m
                             , mcs_type& mcs
                             )
      {
        net_type& n (id_net.get_ref());

        n.contains_a_module_call = false;

        BOOST_FOREACH (transition_type& transition, n.transitions().values())
          {
            n.contains_a_module_call
              |= boost::apply_visitor
              ( transition_find_module_calls_visitor
                (state, id_net, transition, m, mcs)
              , transition.function_or_use()
              );
          }

        return n.contains_a_module_call;
      }

      namespace
      {
        typedef boost::unordered_set<std::string> types_type;

        struct port_with_type
        {
        public:
          std::string name;
          std::string type;

          port_with_type ( const std::string & _name
                         , const std::string & _type
                         )
            : name (_name), type (_type)
          { }

        };

        typedef std::list<port_with_type> ports_with_type_type;

        template<typename Stream>
        void mod_includes (Stream& s, const types_type& types)
        {
          namespace cpp_util = ::fhg::util::cpp;

          for ( types_type::const_iterator type (types.begin())
              ; type != types.end()
              ; ++type
              )
            {
              if (!literal::cpp::known (*type))
                {
                  cpp_util::include
                    (s, cpp_util::path::type() / cpp_util::make::hpp (*type));
                }
              else
                {
                  cpp_util::include (s, literal::cpp::include (*type));
                }
            }
        }

        template<typename Stream>
        void namespace_open (Stream& s, const module_type& mod)
        {
          s << std::endl
            << "namespace pnetc" << std::endl
            << "{" << std::endl
            << "  namespace op" << std::endl
            << "  {" << std::endl
            << "    namespace " << mod.name() << std::endl
            << "    {" << std::endl
            ;
        }

        template<typename Stream>
        void namespace_close (Stream& s, const module_type & mod)
        {
          s << "    } // namespace " << mod.name() << std::endl
            << "  } // namespace op" << std::endl
            << "} // namespace pnetc" << std::endl
            ;
        }

        std::string mk_type (const std::string & type)
        {
          namespace cpp_util = ::fhg::util::cpp;

          return literal::cpp::known (type)
            ? literal::cpp::translate (type)
            : cpp_util::access::make ("", "pnetc", "type", type, type)
            ;
        }

        std::string mk_get ( const port_with_type & port
                           , const std::string & amper = ""
                           )
        {
          namespace cpp_util = ::fhg::util::cpp;

          std::ostringstream os;

          os << mk_type (port.type) << " ";

          if (literal::cpp::known (port.type))
            {
              os << amper << port.name << " ("
                 << "::we::loader::get< " << literal::cpp::translate (port.type) << " >"
                 << "(_pnetc_input, \"" << port.name << "\")"
                 << ")"
                ;
            }
          else
            {
              os << port.name << " ("
                 << cpp_util::access::make ("", "pnetc", "type", port.type, "from_value")
                 << "("
                 << "::we::loader::get< " << cpp_util::access::value_type() << " >"
                 << "(_pnetc_input, \"" << port.name << "\")"
                 << ")"
                 << ")"
                ;
            }

          os << ";" << std::endl;

          return os.str();
        }

        std::string mk_value (const port_with_type & port)
        {
          namespace cpp_util = ::fhg::util::cpp;

          std::ostringstream os;

          if (literal::cpp::known (port.type))
            {
              os << port.name;
            }
          else
            {
              os << cpp_util::access::make ( ""
                                           , "pnetc"
                                           , "type"
                                           , port.type
                                           , "to_value"
                                           )
                 << " (" << port.name << ")"
                ;
            }

          return os.str();
        }

        template<typename Stream>
        void
        mod_signature ( Stream& s
                      , const boost::optional<port_with_type> & port_return
                      , const ports_with_type_type & ports_const
                      , const ports_with_type_type & ports_mutable
                      , const ports_with_type_type & ports_out
                      , const module_type & mod
                      )
        {
          std::ostringstream pre;

          pre << "      "
              << (port_return ? mk_type ((*port_return).type) : "void")
              << " "
              << mod.function()
              << " "
            ;

          s << pre.str() << "(";

          const std::string spre (pre.str());

          std::string white;

          for ( std::string::const_iterator pos (spre.begin())
              ; pos != spre.end()
              ; ++pos
              )
            {
              white.push_back (' ');
            }

          bool first (true);

          for ( ports_with_type_type::const_iterator port (ports_const.begin())
              ; port != ports_const.end()
              ; ++port, first = false
              )
            {
              s << (first ? " " : (white + ", "))
                << "const " << mk_type (port->type) << " & " << port->name
                << std::endl
                ;
            }

          for ( ports_with_type_type::const_iterator port (ports_mutable.begin())
              ; port != ports_mutable.end()
              ; ++port, first = false
              )
            {
              s << (first ? " " : (white + ", "))
                << mk_type (port->type) << " & " << port->name
                << std::endl
                ;
            }

          for ( ports_with_type_type::const_iterator port (ports_out.begin())
              ; port != ports_out.end()
              ; ++port, first = false
              )
            {
              s << (first ? " " : (white + ", "))
                << mk_type (port->type) << " & " << port->name
                << std::endl
                ;
            }

          s << white << ")";
        }

        template<typename Stream>
        void
        mod_wrapper ( Stream& s
                    , const module_type & mod
                    , const path_t file_hpp
                    , const ports_with_type_type & ports_const
                    , const ports_with_type_type & ports_mutable
                    , const ports_with_type_type & ports_out
                    , const boost::optional<port_with_type> & port_return
                    )
        {
          namespace cpp_util = ::fhg::util::cpp;

          cpp_util::include ( s
                            , cpp_util::path::op() / mod.name() / file_hpp
                            );

          namespace_open (s, mod);

          s << "      "
            << "static void " << mod.function()
            << " (void *, const ::we::loader::input_t & _pnetc_input, ::we::loader::output_t & _pnetc_output)"
            << std::endl
            << "      "
            << "{" << std::endl;

          for ( ports_with_type_type::const_iterator port (ports_const.begin())
              ; port != ports_const.end()
              ; ++port
              )
            {
              s << "      "
                << "  const " << mk_get (*port, "& ");
            }

          for ( ports_with_type_type::const_iterator port (ports_mutable.begin())
              ; port != ports_mutable.end()
              ; ++port
              )
            {
              s << "      "
                << "  " << mk_get (*port);
            }

          for ( ports_with_type_type::const_iterator port (ports_out.begin())
              ; port != ports_out.end()
              ; ++port
              )
            {
              s << "      "
                << "  " << mk_type (port->type) << " " << port->name << ";"
                << std::endl;
            }

          s << std::endl;

          s << "      "
            << "  ";

          bool first_put (true);

          if (port_return)
            {
              first_put = false;

              s << "_pnetc_output.bind ("
                << "\"" << (*port_return).name << "\""
                << ", "
                ;

              if (!literal::cpp::known ((*port_return).type))
                {
                  s << cpp_util::access::make ( ""
                                              , "pnetc"
                                              , "type"
                                              , (*port_return).type
                                              , "to_value"
                                              )
                    << " ("
                    ;
                }
            }

          s << cpp_util::access::make ( ""
                                      , "pnetc"
                                      , "op"
                                      , mod.name()
                                      , mod.function()
                                      )
            << " ("
            ;

          bool first_param (true);

          for ( ports_with_type_type::const_iterator port (ports_const.begin())
              ; port != ports_const.end()
              ; ++port, first_param = false
              )
            {
              s << (first_param ? "" : ", ") << port->name;
            }

          for ( ports_with_type_type::const_iterator port (ports_mutable.begin())
              ; port != ports_mutable.end()
              ; ++port, first_param = false
              )
            {
              s << (first_param ? "" : ", ") << port->name;
            }

          for ( ports_with_type_type::const_iterator port (ports_out.begin())
              ; port != ports_out.end()
              ; ++port, first_param = false
              )
            {
              s << (first_param ? "" : ", ") << port->name;
            }

          s << ")";

          if (port_return)
            {
              s << ")";

              if (!literal::cpp::known ((*port_return).type))
                {
                  s << ")";
                }
            }

          s << ";" << std::endl;

          for ( ports_with_type_type::const_iterator port (ports_mutable.begin())
              ; port != ports_mutable.end()
              ; ++port
              )
            {
              if (first_put)
                {
                  s << std::endl;

                  first_put = false;
                }

              s << "      "
                << "  _pnetc_output.bind ("
                << "\"" << port->name << "\""
                << ", " << mk_value (*port)
                << ")"
                << ";"
                << std::endl
                ;
            }

          for ( ports_with_type_type::const_iterator port (ports_out.begin())
              ; port != ports_out.end()
              ; ++port
              )
            {
              if (first_put)
                {
                  s << std::endl;

                  first_put = false;
                }

              s << "      "
                << "  _pnetc_output.bind ("
                << "\"" << port->name << "\""
                << ", " << mk_value (*port)
                << ")"
                << ";"
                << std::endl
                ;
            }

          s << "      "
            << "} // " << mod.function() << std::endl;

          namespace_close (s, mod);
        }
      }
      namespace
      {
        class find_module_calls_visitor
          : public boost::static_visitor<bool>
        {
        private:
          const state::type & state;
          const id::ref::function & _id_function;
          fun_info_map & m;
          mcs_type& mcs;

        public:
          find_module_calls_visitor ( const state::type & _state
                                    , const id::ref::function & id_function
                                    , fun_info_map & _m
                                    , mcs_type& _mcs
                                    )
            : state (_state)
            , _id_function (id_function)
            , m (_m)
            , mcs (_mcs)
          {}

          bool operator () (id::ref::expression &) const
          {
            return false;
          }

          bool operator () (id::ref::net & id) const
          {
            return find_module_calls (state, id, m, mcs);
          }

          bool operator () (id::ref::module & id) const
          {
            namespace cpp_util = ::fhg::util::cpp;

            const module_type& mod (id.get());

            const mcs_type::const_iterator old_map (mcs.find (mod.name()));

            if (old_map != mcs.end())
            {
              const mc_by_function_type::const_iterator old_mc
                (old_map->second.find (mod.function()));

              if (old_mc != old_map->second.end())
              {
                if (old_mc->second == mod)
                {
                  state.warn ( warning::duplicate_external_function
                               (id, old_mc->second.make_reference_id())
                             );
                }
                else
                {
                  throw error::duplicate_external_function
                    (old_mc->second.make_reference_id(), id);
                }
              }
            }

            mcs[mod.name()].insert (std::make_pair (mod.function(), mod));

            ports_with_type_type ports_const;
            ports_with_type_type ports_mutable;
            ports_with_type_type ports_out;
            boost::optional<port_with_type> port_return;
            types_type types;

            if (mod.port_return())
            {
              boost::optional<const id::ref::port&> id_port
                (_id_function.get().get_port_out (*mod.port_return()));

              const port_type& port (id_port->get());

              port_return = port_with_type (*mod.port_return(), port.type());
              types.insert (port.type());
            }

            BOOST_FOREACH (const std::string& name, mod.port_arg())
            {
              if (_id_function.get().is_known_port_inout (name))
              {
                boost::optional<const id::ref::port&> id_port_in
                  (_id_function.get().get_port_in (name));
                boost::optional<const id::ref::port&> id_port_out
                  (_id_function.get().get_port_out (name));

                const port_type& port_in (id_port_in->get());

                if (    mod.port_return()
                   && (*mod.port_return() == port_in.name())
                   )
                {
                  ports_const.push_back (port_with_type (name, port_in.type()));
                  types.insert (port_in.type());
                }
                else
                {
                  ports_mutable.push_back (port_with_type (name, port_in.type()));
                  types.insert (port_in.type());
                }
              }
              else if (_id_function.get().is_known_port_in (name))
              {
                boost::optional<const id::ref::port&> id_port_in
                  (_id_function.get().get_port_in (name));

                const port_type& port_in (id_port_in->get());

                ports_const.push_back (port_with_type (name, port_in.type()));
                types.insert (port_in.type());
              }
              else if (_id_function.get().is_known_port_out (name))
              {
                boost::optional<const id::ref::port&> id_port_out
                  (_id_function.get().get_port_out (name));

                const port_type& port_out (id_port_out->get());

                if (    mod.port_return()
                   && (*mod.port_return() == port_out.name())
                   )
                {
                  // do nothing, it is the return port
                }
                else
                {
                  ports_out.push_back (port_with_type (name, port_out.type()));
                  types.insert (port_out.type());
                }
              }
            }

            const path_t prefix (state.path_to_cpp());
            const path_t path (prefix / cpp_util::path::op() / mod.name());
            const std::string file_hpp (cpp_util::make::hpp (mod.function()));
            const std::string file_cpp
              ( mod.code()
              ? cpp_util::make::cpp (mod.function())
              : cpp_util::make::tmpl (mod.function())
              );

            {
              std::ostringstream stream;

              mod_wrapper ( stream
                          , mod
                          , file_hpp
                          , ports_const
                          , ports_mutable
                          , ports_out
                          , port_return
                          );

              const fun_info_type fun_info ( mod.function()
                                           , stream.str()
                                           , mod.ldflags()
                                           , mod.cxxflags()
                                           , mod.links
                                           , mod.position_of_definition().path()
                                           );

              m[mod.name()].insert (fun_info);
            }

            {
              const path_t file (path / file_hpp);

              util::check_no_change_fstream stream (state, file);

              cpp_util::header_gen_full (stream);
              cpp_util::include_guard_begin
                (stream, "PNETC_OP_" + mod.name() + "_" + mod.function());

              mod_includes (stream, types);

              namespace_open (stream, mod);

              mod_signature ( stream
                            , port_return
                            , ports_const, ports_mutable, ports_out, mod
                            );

              stream << ";" << std::endl;

              namespace_close (stream, mod);

              stream << std::endl;

              cpp_util::include_guard_end
                (stream, "PNETC_OP_" + mod.name() + "_" + mod.function());
            }

            {
              const path_t file (path / file_cpp);

              util::check_no_change_fstream stream (state, file);

              cpp_util::header_gen (stream);

              cpp_util::include ( stream
                                , cpp_util::path::op() / mod.name() / file_hpp
                                );

              BOOST_FOREACH (const std::string& inc, mod.cincludes())
              {
                cpp_util::include (stream, inc);
              }

              if (not mod.code())
              {
                cpp_util::include (stream, "stdexcept");
              }

              namespace_open (stream, mod);

              mod_signature ( stream
                            , port_return
                            , ports_const, ports_mutable, ports_out, mod
                            );

              stream << std::endl << "      {" << std::endl;

              if (not mod.code())
              {
                stream << "        // INSERT CODE HERE" << std::endl
                       << "        throw std::runtime_error (\""
                       << mod.name() << "::" << mod.function()
                       << ": NOT YET IMPLEMENTED\");";
              }
              else
              {
                if (!mod.position_of_definition_of_code())
                {
                  throw std::runtime_error
                    ("STRANGE: There is code without a position of definition");
                }

                stream << "// defined at "
                       << *mod.position_of_definition_of_code()
                       << std::endl;
                stream << *mod.code();
              }

              stream << std::endl << "      }" << std::endl;

              namespace_close (stream, mod);
            }

            return true;
          }
        };
      }

      bool find_module_calls ( const state::type & state
                             , const id::ref::function& id_function
                             , fun_info_map & m
                             )
      {
        mcs_type mcs;

        return find_module_calls (state, id_function, m, mcs);
      }

      bool find_module_calls ( const state::type & state
                             , const id::ref::function & id_function
                             , fun_info_map & m
                             , mcs_type& mcs
                             )
      {
        id_function.get_ref().contains_a_module_call
          = boost::apply_visitor
          ( find_module_calls_visitor (state, id_function, m, mcs)
          , id_function.get_ref().content()
          );

        return id_function.get().contains_a_module_call;
      }

      // **************************************************************** //

      namespace
      {
        void to_cpp (const structs_type& structs, const state::type& state)
        {
          BOOST_FOREACH (const structure_type& structure, structs)
          {
            const boost::filesystem::path prefix (state.path_to_cpp());
            const boost::filesystem::path file
              ( prefix
              / ::fhg::util::cpp::path::type()
              / ::fhg::util::cpp::make::hpp (structure.name())
              );

            util::check_no_change_fstream stream (state, file);

            signature::cpp::cpp_header
              ( stream
              , structure.signature()
              , structure.name()
              , structure.position_of_definition().path()
              , ::fhg::util::cpp::path::type()
              );
          }
        }

        class visitor_to_cpp : public boost::static_visitor<void>
        {
        private:
          const state::type & state;

        public:
          visitor_to_cpp (const state::type & _state)
            : state (_state)
          {}

          void operator() (const id::ref::net& id) const
          {
            const net_type& n (id.get());

            if (n.contains_a_module_call)
            {
              to_cpp (n.structs, state);

              BOOST_FOREACH ( const id::ref::function& id_function
                            , n.functions().ids()
                            )
              {
                struct_to_cpp (state, id_function);
              }

              BOOST_FOREACH ( const transition_type& transition
                            , n.transitions().values()
                            )
              {
                boost::apply_visitor (*this, transition.function_or_use());
              }
            }
          }

          void operator() (const id::ref::function& f) const
          {
            struct_to_cpp (state, f);
          }

          void operator() (const id::ref::use&) const { }
          void operator() (const id::ref::module&) const { }
          void operator() (const id::ref::expression&) const { }
        };
      }

      void struct_to_cpp ( const state::type& state
                         , const id::ref::function& function_id
                         )
      {
        const function_type& function (function_id.get());

        if (function.contains_a_module_call)
        {
          to_cpp (function.structs, state);

          boost::apply_visitor (visitor_to_cpp (state), function.content());
        }
      }

      // **************************************************************** //

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream &
                  , const function_type &
                  );

        void dump ( ::fhg::util::xml::xmlstream &
                  , const transition_type &
                  );

        void dump ( ::fhg::util::xml::xmlstream &
                  , const tmpl_type &
                  );

        namespace
        {
          class function_dump_visitor : public boost::static_visitor<void>
          {
          private:
            ::fhg::util::xml::xmlstream & s;

          public:
            function_dump_visitor (::fhg::util::xml::xmlstream & _s)
              : s (_s)
            {}

            template<typename ID>
            void operator () (const ID& id) const
            {
              ::xml::parse::type::dump::dump (s, id.get());
            }
          };
        }

        void dump ( ::fhg::util::xml::xmlstream & s
                  , const function_type & f
                  )
        {
          s.open ("defun");
          s.attr ("name", f.name());
          s.attr ("internal", f.internal);

          ::we::type::property::dump::dump (s, f.properties());

          BOOST_FOREACH (const std::string& tn, f.typenames())
          {
            s.open ("template-parameter");
            s.attr ("type", tn);
            s.close();
          }

          dumps (s, f.structs.begin(), f.structs.end());

          xml::parse::type::dump::dump (s, f.requirements);

          dumps (s, f.ports().values());

          boost::apply_visitor (function_dump_visitor (s), f.content());

          BOOST_FOREACH (const std::string& cond, f.conditions())
          {
            s.open ("condition");
            s.content (cond);
            s.close();
          }

          s.close ();
        }
      } // namespace dump
    }
  }
}
