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
#include <fhg/util/first_then.hpp>

#include <fhg/util/cpp/block.hpp>
#include <fhg/util/cpp/namespace.hpp>
#include <fhg/util/cpp/include.hpp>
#include <fhg/util/cpp/include_guard.hpp>
#include <fhg/util/indenter.hpp>
#include <fhg/util/ostream_modifier.hpp>

#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/net.fwd.hpp>

#include <we2/type/signature/cpp.hpp>
#include <we2/type/signature/names.hpp>
#include <we2/type/signature/is_literal.hpp>
#include <we2/type/signature/complete.hpp>
#include <we2/type/compat.sig.hpp>

#include <we2/type/value/name.hpp>

#include <we/type/id.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptors.hpp>

#include <iostream>
#include <string>

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

      xml::parse::structure_type_util::forbidden_type
        function_type::forbidden_below (void) const
      {
        xml::parse::structure_type_util::forbidden_type forbidden;

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
                         , boost::bind ( parse::structure_type_util::struct_by_name
                                       , type
                                       , _1
                                       )
                         )
          );

        if (pos != structs.end())
        {
          return signature::type
            ( parse::structure_type_util::resolve_with_fun
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
        const xml::parse::structure_type_util::set_type known_empty;

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
          const xml::parse::structure_type_util::set_type & known_structs;
          state::type & state;
          function_type & fun;

        public:
          function_specialize
            ( const type::type_map_type & _map
            , const type::type_get_type & _get
            , const xml::parse::structure_type_util::set_type & _known_structs
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
                                     , const xml::parse::structure_type_util::set_type & known_structs
                                     , state::type & state
                                     )
      {
        BOOST_FOREACH (port_type& port, ports().values())
        {
          port.specialize (map, state);
        }

        BOOST_FOREACH (structure_type& s, structs)
        {
          s.specialize (map);
        }

        namespace st = xml::parse::structure_type_util;

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
                                   , const std::list<link_type>& _links
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
        fhg::util::indenter indent;

        BOOST_FOREACH (const fun_info_map::value_type& mod, m)
        {
          const std::string& modname (mod.first);
          const fun_infos_type& funs (mod.second);

          util::check_no_change_fstream stream
            ( state
            , state.path_to_cpp() + "/pnetc/op/" + modname + ".cpp"
            );

          stream << fhg::util::cpp::include ("we/loader/macros.hpp");

          BOOST_FOREACH (const fun_info_type& fun, funs)
          {
            stream << std::endl << fun.code;
          }

          stream << std::endl;
          stream << "WE_MOD_INITIALIZE_START (" << modname << ");";
          stream << fhg::util::cpp::block::open (indent);
          BOOST_FOREACH (const fun_info_type& fun, funs)
          {
            stream << indent
                   << "WE_REGISTER_FUN_AS ("
                   << "::pnetc"
                   << "::op"
                   << "::" << modname
                   << "::" << fun.name
                   << ",\"" << fun.name << "\""
                   << ");";
          }
          stream << fhg::util::cpp::block::close (indent) << std::endl;
          stream << "WE_MOD_INITIALIZE_END (" << modname << ");" << std::endl;

          stream << std::endl;
          stream << "WE_MOD_FINALIZE_START (" << modname << ");";
          stream << fhg::util::cpp::block::open (indent);
          stream << fhg::util::cpp::block::close (indent);
          stream << std::endl;
          stream << "WE_MOD_FINALIZE_END (" << modname << ");" << std::endl;
        }
      }

      void mk_makefile ( const state::type & state
                       , const fun_info_map & m
                       , const boost::unordered_set<std::string>& structnames
                       )
      {
        namespace cpp_util = ::fhg::util::cpp;

        const path_t prefix (state.path_to_cpp());
        const path_t file (prefix / "Makefile");

        const std::string path_type ("pnetc/type/");
        const std::string path_op ("pnetc/op/");

        const std::string file_global_cxxflags ("Makefile.CXXFLAGS");
        const std::string file_global_ldflags ("Makefile.LDFLAGS");

        util::check_no_change_fstream stream (state, file);

        stream << "# GPI-Space generated: DO NOT EDIT THIS FILE!"  << std::endl;
        stream                                                     << std::endl;

        BOOST_FOREACH (const fun_info_map::value_type& mod, m)
        {
          stream << "MODULES += pnetc/op/lib" << mod.first << ".so"
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

        BOOST_FOREACH (const std::string& tname, structnames)
        {
          const std::string obj (path_type + tname + ".o");
          const std::string obj_cpp (path_type + tname + ".cpp");
          const std::string dep (path_type + tname + ".d");

          stream << "####"                                         << std::endl;
          stream << "#### struct " << tname                        << std::endl;
          stream << "####"                                         << std::endl;

          stream << "TYPE_OBJS += " << obj << std::endl;

          stream << obj << ": " << obj_cpp << " "
                 << file_global_cxxflags                         << std::endl;
          stream << "\t$(CXX) $(CXXFLAGS) -c $< -o $@"           << std::endl;
          stream << dep << ": " << obj_cpp
                 << " "
                 << file_global_cxxflags                         << std::endl;
          stream << "\t$(CXX) $(CXXFLAGS)"
                 << " -MM -MP -MT '" << dep << "' -MT '"
                 << obj
                 << "' $< -MF $@"                                << std::endl;
          stream << "ifneq \"$(wildcard " << dep << ")\" \"\""   << std::endl;
          stream << "  include " << dep                          << std::endl;
          stream << "endif"                                      << std::endl;
          stream << "DEPENDS += " << dep                         << std::endl;
          stream                                                 << std::endl;
        }

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

                const std::string obj_fun
                  (std::string ("pnetc/op/") + mod->first + "/" + fun->name + ".o");

                stream << objs << " += " << obj_fun << std::endl;

                BOOST_FOREACH (const link_type& link, fun->links)
                  {
                    stream
                      << objs << " += "
                      << ( link.prefix()
                         ? boost::filesystem::absolute
                           ( link.link
                             ( boost::bind ( &state::type::link_prefix_by_key
                                           , boost::ref (state)
                                           , _1
                                           )
                             )
                           , fun->path.parent_path()
                           ).string()
                         : link.href()
                         )
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

                const std::string dep
                  (path_op + mod->first + "/" + fun->name + ".d");
                const std::string cpp
                  (path_op + mod->first + "/" + fun->name + ".cpp");

                stream << obj_fun
                       << ": "
                       << cpp
                       << " "
                       << file_global_cxxflags
                       << " "
                       << file_function_cxxflags
                                                                   << std::endl;
                stream << "\t$(CXX) $(CXXFLAGS)" << " $(" << cxxflags << ")"
                       << " -c $< -o $@"                           << std::endl;
                stream << dep << ": "
                       << cpp
                       << " "
                       << file_global_cxxflags
                       << " "
                       << file_function_cxxflags
                                                                   << std::endl;
                stream << "\t$(CXX) $(CXXFLAGS)" << " $(" << cxxflags << ")"
                       << " -MM -MP -MT '" << dep << "' -MT '"
                       << obj_fun
                       << "' $< -MF $@"                            << std::endl;
                stream << "ifneq \"$(wildcard " << dep << ")\" \"\""
                                                                   << std::endl;
                stream << "  include " << dep                      << std::endl;
                stream << "endif"                                  << std::endl;
                stream << "DEPENDS += " << dep                     << std::endl;
                stream                                             << std::endl;
              }

            stream << "####"                                       << std::endl;
            stream << "#### module " << mod->first                 << std::endl;
            stream << "####"                                       << std::endl;

            const std::string obj (path_op + mod->first + ".o");

            stream << objs << " += " << obj << std::endl;

            const std::string obj_cpp (path_op + mod->first + ".cpp");
            const std::string dep (path_op + mod->first + ".d");

            stream << obj << ": " << obj_cpp << " "
                   << file_global_cxxflags                         << std::endl;
            stream << "\t$(CXX) $(CXXFLAGS) -c $< -o $@"           << std::endl;
            stream << dep << ": " << obj_cpp << " "
                   << file_global_cxxflags                         << std::endl;
            stream << "\t$(CXX) $(CXXFLAGS)"
                   << " -MM -MP -MT '" << dep
                   << "' -MT '" << obj
                   << "' $< -MF $@"                                << std::endl;
            stream << "ifneq \"$(wildcard " << dep << ")\" \"\""   << std::endl;
            stream << "  include " << dep                          << std::endl;
            stream << "endif"                                      << std::endl;
            stream << "DEPENDS += " << dep                         << std::endl;

            stream                                                 << std::endl;
            stream << "include " << file_module_ldflags            << std::endl;
            stream                                                 << std::endl;
            stream << "pnetc/op/lib" << mod->first << ".so"
                   << ": $(" << objs << ")"
                   << " $(TYPE_OBJS) "
                   << file_global_ldflags
                   << " "
                   << file_module_ldflags                          << std::endl;
            stream << "\t$(CXX)"
                   << " -shared "
                   << "$(filter-out"
                   << " " << file_global_ldflags
                   << " " << file_module_ldflags
                   << ", $^)"
                   << " -o $@"
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
            stream << "$(LIB_DESTDIR)/lib" << mod->first << ".so"
                   << ": pnetc/op/lib" << mod->first << ".so"
                   << " $(LIB_DESTDIR)"                            << std::endl;
            stream << "\t@$(CP) -v $< $@"                          << std::endl;
          }

        stream                                                     << std::endl;

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            stream << "MODULES_INSTALL += $(LIB_DESTDIR)/lib"
                   << mod->first << ".so"
                   << std::endl;
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

        namespace
        {
          class include : public fhg::util::ostream::modifier
          {
          public:
            include (const std::string& tname)
              : _tname (tname)
              , _inc()
            {
              _inc[pnet::type::value::CONTROL()] = "we/type/literal/control.hpp";
              _inc[pnet::type::value::STRING()] = "string";
              _inc[pnet::type::value::BITSET()] = "we/type/bitsetofint.hpp";
              _inc[pnet::type::value::BYTEARRAY()] = "we/type/bytearray.hpp";
              _inc[pnet::type::value::LIST()] = "list";
              _inc[pnet::type::value::SET()] = "set";
              _inc[pnet::type::value::MAP()] = "map";
            }

            std::ostream& operator() (std::ostream& os) const
            {
              if (!pnet::type::signature::is_literal (_tname))
              {
                os << fhg::util::cpp::include ("pnetc/type/" + _tname + ".hpp");
              }
              else
              {
                boost::unordered_map<std::string, std::string>::const_iterator
                  pos (_inc.find (_tname));

                if (pos != _inc.end())
                {
                  os << fhg::util::cpp::include (pos->second);
                }
              }

              return os;
            }

          private:
            const std::string _tname;
            boost::unordered_map<std::string, std::string> _inc;
          };
        };

        class mk_type : public fhg::util::ostream::modifier
        {
        public:
          mk_type (const std::string& type)
            : _type (type)
          {}
          std::ostream& operator() (std::ostream& os) const
          {
            if (pnet::type::signature::is_literal (_type))
            {
              os << pnet::type::signature::complete (_type);
            }
            else
            {
              os << "::pnetc::type::" << _type << "::type";
            }

            return os;
          }
        private:
          const std::string& _type;
        };

        class mk_get : public fhg::util::ostream::modifier
        {
        public:
          mk_get ( fhg::util::indenter& indent
                 , const port_with_type& port
                 , const std::string& modif = ""
                 , const std::string& amper = ""
                 )
            : _indent (indent)
            , _port (port)
            , _modif (modif)
            , _amper (amper)
          {}
          std::ostream& operator() (std::ostream& os) const
          {
            os << _indent << _modif;

            if (pnet::type::signature::is_literal (_port.type))
            {
              using pnet::type::signature::complete;

              os << complete (_port.type) << " " << _amper << _port.name << " ("
                 << "boost::get< " << _modif << complete (_port.type) << _amper << " >"
                 << " (_pnetc_input.value (\"" << _port.name << "\")));";
            }
            else
            {
              os << "::pnetc::type::" << _port.type <<  "::type " << _port.name
                 << " (_pnetc_input.value (\"" << _port.name << "\"));";
            }

            return os;
          }

        private:
          fhg::util::indenter& _indent;
          const port_with_type& _port;
          const std::string _modif;
          const std::string _amper;
        };

        std::string mk_value (const port_with_type & port)
        {
          namespace cpp_util = ::fhg::util::cpp;

          std::ostringstream os;

          if (pnet::type::signature::is_literal (port.type))
            {
              os << port.name;
            }
          else
            {
              os << "::pnetc::type::" << port.type << "::value"
                 << " (" << port.name << ")"
                ;
            }

          return os.str();
        }

        template<typename Stream>
        void
        mod_signature ( Stream& s
                      , fhg::util::indenter& indent
                      , const boost::optional<port_with_type> & port_return
                      , const ports_with_type_type & ports_const
                      , const ports_with_type_type & ports_mutable
                      , const ports_with_type_type & ports_out
                      , const module_type & mod
                      )
        {
          using fhg::util::deeper;

          s << indent;

          if (port_return)
          {
            s << mk_type ((*port_return).type);
          }
          else
          {
            s << "void";
          }

          s << " " << mod.function() << deeper (indent) << "(";

          fhg::util::first_then<std::string> sep (" ", ", ");

          BOOST_FOREACH (const port_with_type& port, ports_const)
          {
            s << sep << "const " << mk_type (port.type) << "& " << port.name
              << deeper (indent);
          }

          BOOST_FOREACH (const port_with_type& port, ports_mutable)
          {
            s << sep << mk_type (port.type) << "& " << port.name
              << deeper (indent);
          }

          BOOST_FOREACH (const port_with_type& port, ports_out)
          {
            s << sep << mk_type (port.type) << "& " << port.name
              << deeper (indent);
          }

          s << ")";
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
          namespace block = fhg::util::cpp::block;
          namespace ns = fhg::util::cpp::ns;
          namespace cpp = fhg::util::cpp;
          using fhg::util::deeper;

          fhg::util::indenter indent;

          s << cpp::include ("pnetc/op/" + mod.name() + "/" + file_hpp.string());
          s << ns::open (indent, "pnetc");
          s << ns::open (indent, "op");
          s << ns::open (indent, mod.name());

          s << indent << "static void " << mod.function();
          s << deeper (indent) << "( void *";
          s << deeper (indent) << ", const ::we::loader::input_t& _pnetc_input";
          s << deeper (indent) << ", ::we::loader::output_t& _pnetc_output";
          s << deeper (indent) << ")";
          s << block::open (indent);

          BOOST_FOREACH (const port_with_type& port, ports_const)
          {
            s << mk_get (indent, port, "const ", "& ");
          }

          BOOST_FOREACH (const port_with_type& port, ports_mutable)
          {
            s << mk_get (indent, port);
          }

          BOOST_FOREACH (const port_with_type& port, ports_out)
          {
            s << indent << mk_type (port.type) << " " << port.name << ";";
          }

          if (port_return)
          {
            s << indent << "_pnetc_output.bind ("
              << "\"" << (*port_return).name << "\""
              << ", "
              ;

            if (!pnet::type::signature::is_literal ((*port_return).type))
            {
              s << "::pnetc::type::" << (*port_return).type << "::value"
                << " ("
                ;
            }
          }

          s << indent << "::pnetc::op::" << mod.name() << "::" << mod.function()
            << " ("
            ;

          fhg::util::first_then<std::string> sep ("", ", ");

          BOOST_FOREACH (const port_with_type& port, ports_const)
          {
            s << sep << port.name;
          }

          BOOST_FOREACH (const port_with_type& port, ports_mutable)
          {
            s << sep << port.name;
          }

          BOOST_FOREACH (const port_with_type& port, ports_out)
          {
            s << sep << port.name;
          }

          s << ")";

          if (port_return)
          {
            s << ")";

            if (!pnet::type::signature::is_literal ((*port_return).type))
            {
              s << ")";
            }
          }

          s << ";";

          BOOST_FOREACH (const port_with_type& port, ports_mutable)
          {
            s << indent
              << "_pnetc_output.bind ("
              << "\"" << port.name << "\""
              << ", " << mk_value (port)
              << ")"
              << ";"
              ;
          }

          BOOST_FOREACH (const port_with_type& port, ports_out)
          {
            s << indent
              << "_pnetc_output.bind ("
              << "\"" << port.name << "\""
              << ", " << mk_value (port)
              << ")"
              << ";"
              ;
          }

          s << block::close (indent)
            << ns::close (indent)
            << ns::close (indent)
            << ns::close (indent);
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
            const path_t path (state.path_to_cpp() + "/pnetc/op/" + mod.name());
            const std::string file_hpp (mod.function() + ".hpp");
            const std::string file_cpp
              (mod.function() + (mod.code() ? ".cpp" : ".cpp_tmpl"));

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
                                           , mod.links()
                                           , mod.position_of_definition().path()
                                           );

              m[mod.name()].insert (fun_info);
            }

            {
              namespace ns = fhg::util::cpp::ns;

              fhg::util::indenter indent;

              const path_t file (path / file_hpp);

              util::check_no_change_fstream stream (state, file);

              stream << cpp_util::include_guard::open
                ("PNETC_OP_" + mod.name() + "_" + mod.function());

              BOOST_FOREACH (const std::string& tname, types)
              {
                stream << include (tname);
              }

              stream << ns::open (indent, "pnetc");
              stream << ns::open (indent, "op");
              stream << ns::open (indent, mod.name());

              mod_signature ( stream
                            , indent
                            , port_return
                            , ports_const, ports_mutable, ports_out, mod
                            );

              stream << ";";

              stream << ns::close (indent)
                     << ns::close (indent)
                     << ns::close (indent);

              stream << std::endl;

              stream << cpp_util::include_guard::close();
            }

            {
              namespace ns = fhg::util::cpp::ns;
              namespace block = fhg::util::cpp::block;

              fhg::util::indenter indent;

              const path_t file (path / file_cpp);

              util::check_no_change_fstream stream (state, file);

              stream << cpp_util::include
                ("pnetc/op/" + mod.name() + "/" + file_hpp);

              BOOST_FOREACH (const std::string& inc, mod.cincludes())
              {
                stream << cpp_util::include (inc);
              }

              if (not mod.code())
              {
                stream << cpp_util::include ("stdexcept");
              }

              stream << ns::open (indent, "pnetc");
              stream << ns::open (indent, "op");
              stream << ns::open (indent, mod.name());

              mod_signature ( stream
                            , indent
                            , port_return
                            , ports_const, ports_mutable, ports_out, mod
                            );

              stream << block::open (indent);

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

                stream << indent << "// defined at "
                       << *mod.position_of_definition_of_code()
                       << std::endl;
                stream << *mod.code();
              }

              stream << block::close (indent)
                     << ns::close (indent)
                     << ns::close (indent)
                     << ns::close (indent);
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
        void to_cpp ( const structs_type& structs
                    , const state::type& state
                    , boost::unordered_set<std::string>& structnames
                    )
        {
          BOOST_FOREACH (const structure_type& structure, structs)
          {
            structnames.insert (structure.name());

            const boost::filesystem::path prefix (state.path_to_cpp());

            {
              const boost::filesystem::path file
                ( prefix
                / "pnetc/type"
                / (structure.name() + ".hpp")
                );

              util::check_no_change_fstream stream (state, file);

              const pnet::type::signature::signature_type sig
                (pnet::type::compat::COMPAT ( structure.signature()
                                            , structure.name()
                                            )
                );

              stream << fhg::util::cpp::include_guard::open
                ("PNETC_TYPE_" + structure.name());

              const boost::unordered_set<std::string> names
                (pnet::type::signature::names (sig));

              BOOST_FOREACH (const std::string& tname, names)
              {
                if (!pnet::type::signature::is_literal (tname))
                {
                  stream <<
                    fhg::util::cpp::include ("pnetc/type/" + tname + ".hpp");
                }
              }

              stream << pnet::type::signature::cpp::header_signature (sig)
                     << std::endl;

              stream << fhg::util::cpp::include_guard::close();
            }

            {
              const boost::filesystem::path file
                ( prefix
                / "pnetc/type"
                / (structure.name() + ".cpp")
                );

              util::check_no_change_fstream stream (state, file);

              const pnet::type::signature::signature_type sig
                (pnet::type::compat::COMPAT ( structure.signature()
                                            , structure.name()
                                            )
                );

              stream << "// defined in " << structure.position_of_definition()
                     << std::endl;

              stream << fhg::util::cpp::include ( "pnetc/type/"
                                                + structure.name() + ".hpp"
                                                );

              stream << pnet::type::signature::cpp::impl_signature (sig)
                     << std::endl;
            }
          }
        }

        class visitor_to_cpp : public boost::static_visitor<void>
        {
        public:
          visitor_to_cpp ( const state::type & _state
                         , boost::unordered_set<std::string>& structnames
                         )
            : state (_state)
            , _structnames (structnames)
          {}

          void operator() (const id::ref::net& id) const
          {
            const net_type& n (id.get());

            if (n.contains_a_module_call)
            {
              to_cpp (n.structs, state, _structnames);

              BOOST_FOREACH ( const id::ref::function& id_function
                            , n.functions().ids()
                            )
              {
                struct_to_cpp (state, id_function, _structnames);
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
            struct_to_cpp (state, f, _structnames);
          }

          void operator() (const id::ref::use&) const { }
          void operator() (const id::ref::module&) const { }
          void operator() (const id::ref::expression&) const { }

        private:
          const state::type & state;
          boost::unordered_set<std::string>& _structnames;
        };
      }

      void struct_to_cpp ( const state::type& state
                         , const id::ref::function& function_id
                         , boost::unordered_set<std::string>& structnames
                         )
      {
        const function_type& function (function_id.get());

        if (function.contains_a_module_call)
        {
          to_cpp (function.structs, state, structnames);

          boost::apply_visitor ( visitor_to_cpp (state, structnames)
                               , function.content()
                               );
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
