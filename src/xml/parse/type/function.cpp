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
#include <util-generic/first_then.hpp>

#include <fhg/util/cpp/block.hpp>
#include <fhg/util/cpp/namespace.hpp>
#include <fhg/util/cpp/include.hpp>
#include <fhg/util/cpp/include_guard.hpp>
#include <fhg/util/indenter.hpp>
#include <fhg/util/starts_with.hpp>
#include <util-generic/ostream_modifier.hpp>
#include <util-generic/split.hpp>

#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/net.fwd.hpp>

#include <we/type/signature/cpp.hpp>
#include <we/type/signature/names.hpp>
#include <we/type/signature/is_literal.hpp>
#include <we/type/signature/complete.hpp>
#include <we/type/signature/resolve.hpp>

#include <we/type/value/name.hpp>

#include <we/type/id.hpp>

#include <boost/range/adaptors.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <functional>
#include <iostream>
#include <string>
#include <set>

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
        , xml::util::unique<memory_buffer_type, id::ref::memory_buffer> const& memory_buffers
        , std::list<memory_get> const& memory_gets
        , std::list<memory_put> const& memory_puts
        , std::list<memory_getput> const& memory_getputs
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
        , _memory_buffers (memory_buffers, _id)
        , _memory_gets (memory_gets)
        , _memory_puts (memory_puts)
        , _memory_getputs (memory_getputs)
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
        return !!_parent;
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
          : public boost::static_visitor<boost::optional<id_ref_type>>
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
          : public boost::static_visitor<boost::optional<const id::ref::function&>>
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

      void function_type::push_memory_get (memory_get const& mg)
      {
        _memory_gets.push_back (mg);
      }
      void function_type::push_memory_put (memory_put const& mp)
      {
        _memory_puts.push_back (mp);
      }
      void function_type::push_memory_getput (memory_getput const& mgp)
      {
        _memory_getputs.push_back (mgp);
      }

      std::list<memory_get> const& function_type::memory_gets() const
      {
        return _memory_gets;
      }
      std::list<memory_put> const& function_type::memory_puts() const
      {
        return _memory_puts;
      }
      std::list<memory_getput> const& function_type::memory_getputs() const
      {
        return _memory_getputs;
      }

      void function_type::push_memory_buffer
        (id::ref::memory_buffer const& id)
      {
        id::ref::memory_buffer const& id_old (_memory_buffers.push (id));

        if (id_old != id)
        {
          throw error::duplicate_memory_buffer (id_old, id);
        }

        id.get_ref().parent (_id);

        {
          boost::optional<id::ref::port const&> port_in
            (get_port_in (id.get().name()));

          if (port_in)
          {
            throw error::memory_buffer_with_same_name_as_port (id, *port_in);
          }
        }

        {
          boost::optional<id::ref::port const&> port_out
            (get_port_out (id.get().name()));

          if (port_out)
          {
            throw error::memory_buffer_with_same_name_as_port (id, *port_out);
          }
        }
      }

      xml::util::unique<memory_buffer_type, id::ref::memory_buffer>
        const& function_type::memory_buffers() const
      {
        return _memory_buffers;
      }

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

        boost::optional<id::ref::memory_buffer const&>
          memory_buffer (memory_buffers().get (id.get().name()));

        if (memory_buffer)
        {
          throw error::memory_buffer_with_same_name_as_port
            (*memory_buffer, id);
        }
      }

      bool function_type::is_known_memory_buffer
        (std::string const& name) const
      {
        return _memory_buffers.has (name);
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
        return !!get_port_in (name);
      }

      bool function_type::is_known_port_out (const std::string & name) const
      {
        return !!get_port_out (name);
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

        for (const port_type& port : ports().values())
        {
          forbidden.emplace (port.type(), port.name());
        }

        return forbidden;
      }

      // ***************************************************************** //

      boost::optional<pnet::type::signature::signature_type>
      function_type::signature (const std::string& type) const
      {
        const structs_type::const_iterator pos
          ( std::find_if ( structs.begin()
                         , structs.end()
                         , std::bind ( parse::structure_type_util::struct_by_name
                                     , type
                                     , std::placeholders::_1
                                     )
                         )
          );

        if (pos != structs.end())
        {
          return pnet::type::signature::resolve
            ( pos->signature()
            , std::bind (&function_type::signature, *this, std::placeholders::_1)
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
        for (const port_type& port : ports().values())
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
        std::unordered_map<std::string, we::port_id_type>& _port_id_in;
        std::unordered_map<std::string, we::port_id_type>& _port_id_out;
        we::priority_type _priority;
        xml::util::range_type<place_map_type const> _place_map;
        std::unordered_map<we::port_id_type, std::string>& _real_place_names;

        typedef we::type::transition_t we_transition_type;

        typedef we::type::net_type we_net_type;
        typedef we::type::module_call_t we_module_type;
        typedef we::type::expression_t we_expr_type;

        typedef std::unordered_map< std::string
                                  , we::place_id_type
                                  > pid_of_place_type;

        we::port_id_type add_port
          (we_transition_type& transition, we::type::port_t const& port) const
        {
          we::port_id_type const port_id (transition.add_port (port));

          if (port.is_output())
          {
            _port_id_out.emplace (port.name(), port_id);
          }
          else
          {
            _port_id_in.emplace (port.name(), port_id);
          }

          return port_id;
        }

        void add_ports ( we_transition_type & trans
                       , const function_type::ports_type& ports
                       ) const
        {
          for (const port_type& port : ports.values())
          {
            add_port (trans, we::type::port_t ( port.name()
                                              , port.direction()
                                              , port.signature_or_throw()
                                              , port.properties()
                                              )
                     );
          }
        }

        template<typename Map>
        void add_ports ( we_transition_type & trans
                       , const function_type::ports_type& ports
                       , const Map & pid_of_place
                       ) const
        {
          for (const port_type& port : ports.values())
          {
            if (not port.place)
            {
              add_port (trans, we::type::port_t ( port.name()
                                                , port.direction()
                                                , port.signature_or_throw()
                                                , port.properties()
                                                )
                       );
            }
            else
            {
              // basically safe, since type checking has verified
              // the existence and type safety of the place to
              // connect to

              we::port_id_type const port_id
                ( add_port ( trans
                           , we::type::port_t ( port.name()
                                              , port.direction()
                                              , port.signature_or_throw()
                                              , pid_of_place.at (*port.place)
                                              , port.properties()
                                              )
                           )
                );

              if (port.direction() == we::type::PORT_TUNNEL)
              {
                for (place_map_type const& place_map : _place_map)
                {
                  if (  place_map.place_virtual()
                     == port.resolved_place()->get().name()
                     )
                  {
                    _real_place_names.emplace (port_id, place_map.place_real());
                  }
                }
              }
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
            trans.add_requirement
              (we::type::requirement_t (r->first, r->second));
          }
        }

        const std::string& name() const
        {
          return _name;
        }

        we::type::expression_t condition (void) const
        {
          const std::string cond ((fun.conditions() + _conditions).flatten());

          expr::parse::parser parsed_condition
            (util::we_parse (cond, "condition", "function", name(), fun.position_of_definition().path()));

          return we::type::expression_t (cond, parsed_condition);
        }

      public:
        function_synthesize
          ( const std::string& name
          , const state::type& _state
          , const function_type& _fun
          , const boost::optional<bool>& internal
          , const conditions_type& conditions
          , const we::type::property::type& trans_properties
          , const requirements_type& trans_requirements
          , std::unordered_map<std::string, we::port_id_type>& port_id_in
          , std::unordered_map<std::string, we::port_id_type>& port_id_out
          , we::priority_type priority
          , xml::util::range_type<place_map_type const> place_map
          , std::unordered_map<we::port_id_type, std::string>& real_place_names
          )
          : _name (name)
          , state (_state)
          , fun (_fun)
          , _internal (internal)
          , _conditions (conditions)
          , _properties (trans_properties)
          , _trans_requirements (trans_requirements)
          , _port_id_in (port_id_in)
          , _port_id_out (port_id_out)
          , _priority (priority)
          , _place_map (place_map)
          , _real_place_names (real_place_names)
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
            , _priority
            );

          add_ports (trans, fun.ports());
          add_requirements (trans);

          return trans;
        }

        we_transition_type operator () (const id::ref::module& id_mod) const
        {
          const module_type& mod (id_mod.get());

          std::unordered_map<std::string, std::string> memory_buffers;

          for (std::string const& memory_buffer_name : mod.memory_buffer_arg())
          {
            id::ref::memory_buffer const& id_memory_buffer
              (*fun.memory_buffers().get (memory_buffer_name));

            memory_buffers.emplace ( id_memory_buffer.get().name()
                                   , id_memory_buffer.get().size()
                                   );
          }

          std::list<we::type::memory_transfer> memory_gets;
          std::list<we::type::memory_transfer> memory_puts;

          for (memory_get const& mg : fun.memory_gets())
          {
            memory_gets.emplace_back (mg.global(), mg.local(), boost::none);
          }
          for (memory_put const& mp : fun.memory_puts())
          {
            memory_puts.emplace_back
              (mp.global(), mp.local(), mp.not_modified_in_module_call());
          }
          for (memory_getput const& mgp : fun.memory_getputs())
          {
            memory_gets.emplace_back (mgp.global(), mgp.local(), boost::none);
            memory_puts.emplace_back
              (mgp.global(), mgp.local(), mgp.not_modified_in_module_call());
          }

          we_transition_type trans
            ( name()
            , we_module_type ( mod.name()
                             , mod.function()
                             , std::move (memory_buffers)
                             , std::move (memory_gets)
                             , std::move (memory_puts)
                             )
            , condition()
            , _internal.get_value_or (false)
            , _properties
            , _priority
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
            , _priority
            );

          add_ports (trans, fun.ports(), pid_of_place);
          add_requirements (trans);

          return trans;
        }
      };

      we::type::transition_t function_type::synthesize
        ( const std::string& name
        , const state::type& state
        , std::unordered_map<std::string, we::port_id_type>& port_id_in
        , std::unordered_map<std::string, we::port_id_type>& port_id_out
        , const boost::optional<bool>& trans_internal
        , const conditions_type& conditions
        , const we::type::property::type& trans_properties
        , const requirements_type& trans_requirements
        , we::priority_type priority
        , xml::util::range_type<place_map_type const> place_map
        , std::unordered_map<we::port_id_type, std::string>& real_place_names
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
                                , port_id_in
                                , port_id_out
                                , priority
                                , place_map
                                , real_place_names
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
        for (port_type& port : _ports.values())
        {
          port.specialize (map, state);
        }

        for (structure_type& s : structs)
        {
          s.specialize (map);
        }

        namespace st = xml::parse::structure_type_util;

        boost::apply_visitor
          ( function_specialize
            ( map
            , get
            , st::join (known_structs, st::make (structs, state), state)
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
          , _memory_buffers.clone (new_id, new_mapper)
          , _memory_gets
          , _memory_puts
          , _memory_getputs
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

      typedef std::unordered_set<fun_info_type> fun_infos_type;

      typedef std::unordered_map<std::string,fun_infos_type> fun_info_map;

      typedef boost::filesystem::path path_t;

      void mk_wrapper ( const state::type & state
                      , const fun_info_map & m
                      )
      {
        fhg::util::indenter indent;

        for (const fun_info_map::value_type& mod : m)
        {
          const std::string& modname (mod.first);
          const fun_infos_type& funs (mod.second);

          util::check_no_change_fstream stream
            ( state
            , state.path_to_cpp() + "/pnetc/op/" + modname + ".cpp"
            );

          stream << fhg::util::cpp::include ("we/loader/macros.hpp");

          for (const fun_info_type& fun : funs)
          {
            stream << std::endl << fun.code;
          }

          stream << std::endl;
          stream << "WE_MOD_INITIALIZE_START (" << modname << ");";
          stream << fhg::util::cpp::block::open (indent);
          for (const fun_info_type& fun : funs)
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
        }
      }

      void mk_makefile ( const state::type & state
                       , const fun_info_map & m
                       , const std::unordered_set<std::string>& structnames
                       )
      {
        namespace cpp_util = ::fhg::util::cpp;

        const path_t prefix (state.path_to_cpp());
        const path_t file (prefix / "Makefile");

        const std::string path_op ("pnetc/op/");

        const std::string file_global_cxxflags ("Makefile.CXXFLAGS");
        const std::string file_global_ldflags ("Makefile.LDFLAGS");

        boost::filesystem::create_directories (prefix);

        if (not boost::filesystem::is_directory (prefix))
        {
          throw error::could_not_create_directory (prefix);
        }

        std::ofstream stream (file.string().c_str());

        if (!stream.good())
        {
          throw error::could_not_open_file (file);
        }

        stream << "# GPI-Space generated: DO NOT EDIT THIS FILE!"  << std::endl;
        stream << "# time of creation: "
               << boost::posix_time::to_simple_string
                  (boost::posix_time::second_clock::local_time())
                                                                   << std::endl;
        stream                                                     << std::endl;

        for (const fun_info_map::value_type& mod : m)
        {
          stream << "MODULES += pnetc/op/lib" << mod.first << ".so"
                 << std::endl;
        }

        stream                                                     << std::endl;
        stream << "CXXFLAGS += -fPIC"                              << std::endl;
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
        stream << "    SDPA_INCLUDE := $(SDPA_HOME)/include"       << std::endl;
        stream << "  endif"                                        << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef BOOST_ROOT"                              << std::endl;
        stream << "  ifndef SDPA_HOME"                             << std::endl;
        stream << "    $(error Neither BOOST_ROOT nor SDPA_HOME are set)"
                                                                   << std::endl;
        stream << "  else"                                         << std::endl;
        stream << "    BOOST_ROOT := $(SDPA_HOME)/external/boost"  << std::endl;
        stream << "  endif"                                        << std::endl;
        stream << "else"                                           << std::endl;
        stream << "  ifdef SDPA_HOME"                              << std::endl;
        stream << "    ifneq \"$(BOOST_ROOT)\" \"$(SDPA_HOME)/external/boost\""
                                                                   << std::endl;
        stream << "      $(warning !!!)"                           << std::endl;
        stream << "      $(warning !!! BOOST_ROOT is set and different from GSPC bundled version)"
                                                                   << std::endl;
        stream << "      $(warning !!! BOOST_ROOT = $(BOOST_ROOT))"    << std::endl;
        stream << "      $(warning !!! GSPC_BUNDLED = $(SDPA_HOME)/external/boost)"
                                                                   << std::endl;
        stream << "      $(warning !!!)"                           << std::endl;
        stream << "    endif"                                      << std::endl;
        stream << "  endif"                                        << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "CXXFLAGS += -I."                                << std::endl;
        stream << "CXXFLAGS += -isystem $(SDPA_INCLUDE)"           << std::endl;
        stream << "CXXFLAGS += -isystem $(BOOST_ROOT)/include"     << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef SDPA_LDPATH"                             << std::endl;
        stream << "  ifndef SDPA_HOME"                             << std::endl;
        stream << "    $(error Neither SDPA_LDPATH nor SDPA_HOME are set)"
                                                                   << std::endl;
        stream << "  else"                                         << std::endl;
        stream << "    SDPA_LDPATH := $(SDPA_HOME)/lib"            << std::endl;
        stream << "  endif"                                        << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "LDFLAGS += -L$(SDPA_LDPATH)"                    << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef CP"                                      << std::endl;
        stream << "  CP := $(shell which cp 2>/dev/null)"          << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef MKDIR"                                   << std::endl;
        stream << "  MKDIR := $(shell which mkdir 2>/dev/null)"    << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef RM"                                      << std::endl;
        stream << "  RM := $(shell which rm 2>/dev/null) -f"       << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;

        {
          util::check_no_change_fstream cxx
            (state, prefix / file_global_cxxflags);

          for (std::string const& flag : state.gen_cxxflags())
          {
            cxx << "CXXFLAGS += " << flag << std::endl;
          }
        }

        {
          util::check_no_change_fstream ld
            (state, prefix / file_global_ldflags);

          for (std::string const& flag : state.gen_ldflags())
          {
            ld << "LDFLAGS += " << flag << std::endl;
          }
        }

        stream << "include " << file_global_cxxflags               << std::endl;
        stream << "include " << file_global_ldflags                << std::endl;

        stream                                                     << std::endl;
        stream << ".PHONY: default modules depend install uninstall"
                                                                   << std::endl;
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

        for (const std::string& tname : structnames)
        {
          const std::string path_type ("pnetc/type/");
          const std::string obj (path_type + tname + "/op.o");
          const std::string obj_cpp (path_type + tname + "/op.cpp");
          const std::string dep (path_type + tname + "/op.d");

          stream << "####"                                         << std::endl;
          stream << "#### struct " << tname                        << std::endl;
          stream << "####"                                         << std::endl;

          stream << "TYPE_OBJS += " << obj << std::endl;

          stream << "-include " << dep                           << std::endl;
          stream << obj << ": " << obj_cpp << " "
                 << file_global_cxxflags                         << std::endl;
          stream << "\t$(CXX) $(CXXFLAGS)"
                 << " -MM -MP -MT '" << obj << "' "
                 << "$< -MF " << dep                             << std::endl;
          stream << "\t$(CXX) $(CXXFLAGS) -c $< -o $@"           << std::endl;
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

                for (const link_type& link : fun->links)
                  {
                    stream
                      << objs << " += "
                      << ( link.prefix()
                         ? boost::filesystem::absolute
                           ( link.link
                             ( std::bind ( &state::type::link_prefix_by_key
                                         , std::ref (state)
                                         , std::placeholders::_1
                                         )
                             )
                           , fun->path.parent_path()
                           ).string()
                         : link.href()
                         )
                      << std::endl;
                  }

                for (const std::string& flag : fun->ldflags)
                  {
                    ldflags_module << ldflags << " += " << flag << std::endl;
                  }

                {
                  util::check_no_change_fstream cxxflags_function
                    (state, prefix / file_function_cxxflags);

                  for (const std::string& flag : fun->cxxflags)
                  {
                    cxxflags_function << cxxflags << " += " << flag << std::endl;
                  }
                }

                stream << "include " << file_function_cxxflags     << std::endl;

                const std::string dep
                  (path_op + mod->first + "/" + fun->name + ".d");
                const std::string cpp
                  (path_op + mod->first + "/" + fun->name + ".cpp");

                stream << "-include " << dep                      << std::endl;
                stream << obj_fun
                       << ": "
                       << cpp
                       << " "
                       << file_global_cxxflags
                       << " "
                       << file_function_cxxflags
                                                                   << std::endl;
                stream << "\t$(CXX) $(CXXFLAGS)" << " $(" << cxxflags << ")"
                       << " -MM -MP -MT '" << obj_fun << "' "
                       << "$< -MF " << dep                         << std::endl;
                stream << "\t$(CXX) $(CXXFLAGS)" << " $(" << cxxflags << ")"
                       << " -c $< -o $@"                           << std::endl;
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
                   << " $(LDFLAGS)"
                   << " -lwe-dev"                                  << std::endl;
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
        stream << "uninstall:"                                     << std::endl;
        stream << "\t-$(RM) $(MODULES_INSTALL)"                    << std::endl;

        stream                                                     << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "endif"                                          << std::endl;
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

      typedef std::unordered_map<std::string, module_type>
        mc_by_function_type;
      typedef std::unordered_map<std::string, mc_by_function_type>
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

        for (const transition_type& transition : n.transitions().values())
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
        typedef std::unordered_set<std::string> types_type;

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
            include ( const std::string& tname
                    , std::string const& suffix
                    )
              : _tname (tname)
              , _inc()
              , _suffix (suffix)
            {
              namespace value = pnet::type::value;

              _inc[value::CONTROL()].insert ("we/type/literal/control.hpp");
              _inc[value::STRING()].insert ("string");
              _inc[value::BITSET()].insert ("we/type/bitsetofint.hpp");
              _inc[value::BYTEARRAY()].insert ("we/type/bytearray.hpp");
              _inc[value::LIST()].insert ("list");
              _inc[value::LIST()].insert ("we/type/value.hpp");
              _inc[value::SET()].insert ("set");
              _inc[value::SET()].insert ("we/type/value.hpp");
              _inc[value::MAP()].insert ("map");
              _inc[value::MAP()].insert ("we/type/value.hpp");
            }

            virtual std::ostream& operator() (std::ostream& os) const override
            {
              if (!pnet::type::signature::is_literal (_tname))
              {
                os << fhg::util::cpp::include ("pnetc/type/" + _tname + _suffix);
              }
              else
              {
                std::unordered_map< std::string
                                  , std::set<std::string>
                                  >::const_iterator
                  pos (_inc.find (_tname));

                if (pos != _inc.end())
                {
                  for (std::string const& i : pos->second)
                  {
                    os << fhg::util::cpp::include (i);
                  }
                }
              }

              return os;
            }

          private:
            const std::string _tname;
            std::unordered_map< std::string
                              , std::set<std::string>
                              > _inc;
            std::string const _suffix;
          };
        };

        class mk_type : public fhg::util::ostream::modifier
        {
        public:
          mk_type (const std::string& type)
            : _type (type)
          {}
          virtual std::ostream& operator() (std::ostream& os) const override
          {
            if (pnet::type::signature::is_literal (_type))
            {
              os << pnet::type::signature::complete (_type);
            }
            else
            {
              os << "::pnetc::type::" << _type << "::" << _type << "";
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
          virtual std::ostream& operator() (std::ostream& os) const override
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
              os << "::pnetc::type::" << _port.type <<  "::" << _port.type <<  " " << _port.name
                 << " (::pnetc::type::" << _port.type << "::from_value (_pnetc_input.value (\"" << _port.name << "\")));";
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
              os << "::pnetc::type::" << port.type << "::to_value"
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
                      , id::ref::function const& id_function
                      )
        {
          using fhg::util::deeper;

          s << indent;

          if (port_return)
          {
            s << mk_type ((*port_return).type);
          }
          else if (mod.memory_buffer_return())
          {
            s << "void*";
          }
          else
          {
            s << "void";
          }

          s << " " << mod.function() << deeper (indent) << "(";

          fhg::util::first_then<std::string> sep (" ", ", ");

          if (mod.pass_context ())
          {
            s << sep << "drts::worker::context *_pnetc_context";
          }

          for (const port_with_type& port : ports_const)
          {
            s << sep << "const " << mk_type (port.type) << "& " << port.name
              << deeper (indent);
          }

          for (const port_with_type& port : ports_mutable)
          {
            s << sep << mk_type (port.type) << "& " << port.name
              << deeper (indent);
          }

          for (const port_with_type& port : ports_out)
          {
            s << sep << mk_type (port.type) << "& " << port.name
              << deeper (indent);
          }

          for (std::string const& memory_buffer_name : mod.memory_buffer_arg())
          {
            id::ref::memory_buffer const& id_memory_buffer
              (*id_function.get().memory_buffers().get (memory_buffer_name));

            s << sep << "void";

            if (id_memory_buffer.get().read_only())
            {
              s << " const";
            }

            s << "* " << id_memory_buffer.get().name() << deeper (indent);
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
                    , std::unordered_set<std::string> const& types
                    , id::ref::function const& id_function
                    )
        {
          namespace block = fhg::util::cpp::block;
          namespace ns = fhg::util::cpp::ns;
          namespace cpp = fhg::util::cpp;
          using fhg::util::deeper;

          fhg::util::indenter indent;

          s << cpp::include ("pnetc/op/" + mod.name() + "/" + file_hpp.string());

          for (std::string const& tname : types)
          {
            s << include (tname, "/op.hpp");
          }

          s << ns::open (indent, "pnetc");
          s << ns::open (indent, "op");
          s << ns::open (indent, mod.name());

          s << indent << "static void " << mod.function();
          s << deeper (indent) << "( drts::worker::context *_pnetc_context";
          s << deeper (indent) << ", const expr::eval::context& _pnetc_input";
          s << deeper (indent) << ", expr::eval::context& _pnetc_output";
          s << deeper (indent) << ", std::map<std::string, void*> const& _pnetc_memory_buffer";
          s << deeper (indent) << ")";
          s << block::open (indent);

          for (const port_with_type& port : ports_const)
          {
            s << mk_get (indent, port, "const ", "& ");
          }

          for (const port_with_type& port : ports_mutable)
          {
            s << mk_get (indent, port);
          }

          for (const port_with_type& port : ports_out)
          {
            s << indent << mk_type (port.type) << " " << port.name << ";";
          }

          if (port_return)
          {
            s << indent << "_pnetc_output.bind_and_discard_ref ("
              << "\"" << (*port_return).name << "\""
              << ", "
              ;

            if (!pnet::type::signature::is_literal ((*port_return).type))
            {
              s << "::pnetc::type::" << (*port_return).type << "::to_value"
                << " ("
                ;
            }
          }

          s << indent << "::pnetc::op::" << mod.name() << "::" << mod.function()
            << " ("
            ;

          fhg::util::first_then<std::string> sep ("", ", ");

          if (mod.pass_context ())
          {
            s << sep << "_pnetc_context";
          }

          for (const port_with_type& port : ports_const)
          {
            s << sep << port.name;
          }

          for (const port_with_type& port : ports_mutable)
          {
            s << sep << port.name;
          }

          for (const port_with_type& port : ports_out)
          {
            s << sep << port.name;
          }

          for (std::string const& memory_buffer_name : mod.memory_buffer_arg())
          {
            id::ref::memory_buffer const& id_memory_buffer
              (*id_function.get().memory_buffers().get (memory_buffer_name));

            s << sep << "_pnetc_memory_buffer.at (\"" << id_memory_buffer.get().name() << "\")";
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

          for (const port_with_type& port : ports_mutable)
          {
            s << indent
              << "_pnetc_output.bind_and_discard_ref ("
              << "\"" << port.name << "\""
              << ", " << mk_value (port)
              << ")"
              << ";"
              ;
          }

          for (const port_with_type& port : ports_out)
          {
            s << indent
              << "_pnetc_output.bind_and_discard_ref ("
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

            mcs[mod.name()].emplace (mod.function(), mod);

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

            for (const std::string& name : mod.port_arg())
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
                          , types
                          , _id_function
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

              if (mod.pass_context ())
              {
                stream << cpp_util::include ("drts/worker/context_fwd.hpp");
              }
              for (const std::string& tname : types)
              {
                stream << include (tname, ".hpp");
              }

              stream << ns::open (indent, "pnetc");
              stream << ns::open (indent, "op");
              stream << ns::open (indent, mod.name());

              mod_signature ( stream
                            , indent
                            , port_return
                            , ports_const, ports_mutable, ports_out, mod
                            , _id_function
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

              util::check_no_change_fstream stream
                ( state
                , file
                , [] (std::string const& l, std::string const& r)
                  {
                    std::list<std::string> const
                      ls (fhg::util::split<std::string, std::string> (l, '\n'));
                    std::list<std::string> const
                      rs (fhg::util::split<std::string, std::string> (r, '\n'));

                    std::list<std::string>::const_iterator pos_l (ls.begin());
                    std::list<std::string>::const_iterator pos_r (rs.begin());

                    while (pos_l != ls.end() && pos_r != rs.end())
                    {
                      if (! ((  fhg::util::starts_with ("#line", *pos_l)
                             && fhg::util::starts_with ("#line", *pos_r)
                             )
                            || *pos_l == *pos_r
                            )
                         )
                      {
                        return false;
                      }

                      ++pos_l;
                      ++pos_r;
                    }

                    return pos_l == ls.end() && pos_r == rs.end();
                  }
                );

              stream << cpp_util::include
                ("pnetc/op/" + mod.name() + "/" + file_hpp);

              for (const std::string& tname : types)
              {
                stream << include (tname, "/op.hpp");
              }

              if (mod.pass_context ())
              {
                stream << cpp_util::include ("drts/worker/context.hpp");
              }

              for (const std::string& inc : mod.cincludes())
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
                            , _id_function
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

                stream << std::endl
                       << "#line "
                       << mod.position_of_definition_of_code()->line()
                       << " "
                       << state.strip_path_prefix
                          (mod.position_of_definition_of_code()->path())
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
                    , std::unordered_set<std::string>& structnames
                    )
        {
          for (const structure_type& structure : structs)
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
                (structure.signature());

              stream << "// " << pnet::type::signature::show (sig) << std::endl;

              stream << fhg::util::cpp::include_guard::open
                ("PNETC_TYPE_" + structure.name());

              const std::unordered_set<std::string> names
                (pnet::type::signature::names (sig));


              for (const std::string& tname : names)
              {
                stream << include (tname, ".hpp");
              }

              stream << pnet::type::signature::cpp::header_signature (sig)
                     << std::endl;

              stream << fhg::util::cpp::include_guard::close();
            }

            {
              const boost::filesystem::path file
                ( prefix
                / "pnetc/type"
                / structure.name()
                / "op.hpp"
                );

              util::check_no_change_fstream stream (state, file);

              const pnet::type::signature::signature_type sig
                (structure.signature());

              stream << "// defined in " << structure.position_of_definition()
                     << std::endl;

              stream << fhg::util::cpp::include_guard::open
                ("PNETC_TYPE_" + structure.name() + "_OP");

              stream << fhg::util::cpp::include ( "pnetc/type/"
                                                + structure.name() + ".hpp"
                                                );

              for (const std::string& tname : pnet::type::signature::names (sig))
              {
                stream << include (tname, "/op.hpp");
              }

              stream << pnet::type::signature::cpp::header_op_signature (sig)
                     << std::endl;

              stream << fhg::util::cpp::include_guard::close();
            }

            {
              const boost::filesystem::path file
                ( prefix
                / "pnetc/type"
                / structure.name()
                / "op.cpp"
                );

              util::check_no_change_fstream stream (state, file);

              const pnet::type::signature::signature_type sig
                (structure.signature());

              stream << "// defined in " << structure.position_of_definition()
                     << std::endl
                     << std::endl;

              stream << fhg::util::cpp::include ( "pnetc/type/"
                                                + structure.name() + "/op.hpp"
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
                         , std::unordered_set<std::string>& structnames
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

              for (const id::ref::function& id_function : n.functions().ids())
              {
                struct_to_cpp (state, id_function, _structnames);
              }

              for (const transition_type& transition : n.transitions().values())
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
          std::unordered_set<std::string>& _structnames;
        };
      }

      void struct_to_cpp ( const state::type& state
                         , const id::ref::function& function_id
                         , std::unordered_set<std::string>& structnames
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

          for (const std::string& tn : f.typenames())
          {
            s.open ("template-parameter");
            s.attr ("type", tn);
            s.close();
          }

          dumps (s, f.structs.begin(), f.structs.end());

          xml::parse::type::dump::dump (s, f.requirements);

          dumps (s, f.ports().values());
          dumps (s, f.memory_buffers().values());
          dumps (s, f.memory_gets());
          dumps (s, f.memory_puts());
          dumps (s, f.memory_getputs());

          boost::apply_visitor (function_dump_visitor (s), f.content());

          for (const std::string& cond : f.conditions())
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

namespace std
{
  size_t hash<xml::parse::type::fun_info_type>::operator()
    (xml::parse::type::fun_info_type const& fun) const
  {
    return std::hash<std::string>() (fun.name);
  }
}
