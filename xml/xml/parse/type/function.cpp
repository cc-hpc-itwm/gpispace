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
#include <xml/parse/util/property.hpp>
#include <xml/parse/util/weparse.hpp>

#include <xml/parse/type/dumps.hpp>

#include <fhg/util/cpp.hpp>

#include <we/type/literal/cpp.hpp>
#include <we/type/signature/cpp.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      // ******************************************************************* //

      function_type::function_type ( ID_CONS_PARAM(function)
                                   , const boost::optional<parent_id_type>& parent
                                   , const type& _f
                                   )
        : ID_INITIALIZE()
        , _parent (parent)
        , contains_a_module_call (false)
        , f (_f)
      {
        _id_mapper->put (_id, *this);
      }

      function_type::function_type
        ( ID_CONS_PARAM(function)
        , const boost::optional<parent_id_type>& parent
        , const boost::optional<std::string>& name
        , const ports_type& in
        , const ports_type& out
        , const ports_type& tunnel
        , const typenames_type& typenames
        , const bool& contains_a_module_call
        , const boost::optional<bool>& internal
        , const structs_type& structs
        , const conditions_type& cond
        , const requirements_type& requirements
        , const type& f
        , const xml::parse::structure_type::set_type& structs_resolved
        , const we::type::property::type& properties
        , const boost::filesystem::path& path
        )
        : ID_INITIALIZE()
        , _parent (parent)
        , _name (name)
        , _in (in)
        , _out (out)
        , _tunnel (tunnel)
        , _typenames (typenames)
        , contains_a_module_call (contains_a_module_call)
        , internal (internal)
        , structs (structs)
        , cond (cond)
        , requirements (requirements)
        , f (f)
        , structs_resolved (structs_resolved)
        , _properties (properties)
        , path (path)
      {
        _id_mapper->put (_id, *this);
      }

      // ******************************************************************* //

      namespace
      {
        class function_is_net_visitor : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const id::ref::net &) const { return true; }
          template<typename T>
          bool operator () (const T &) const { return false; }
        };
      }

      bool function_type::is_net() const
      {
        return boost::apply_visitor (function_is_net_visitor(), f);
      }

      // ******************************************************************* //

      const boost::optional<std::string>& function_type::name() const
      {
        return _name;
      }
      const std::string& function_type::name (const std::string& name)
      {
        return *(_name = name);
      }
      const boost::optional<std::string>&
        function_type::name (const boost::optional<std::string>& name)
      {
        return _name = name;
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

      const function_type::ports_type& function_type::in() const
      {
        return _in;
      }
      const function_type::ports_type& function_type::out() const
      {
        return _out;
      }
      const function_type::ports_type& function_type::tunnel() const
      {
        return _tunnel;
      }

      // ***************************************************************** //

      const function_type::typenames_type& function_type::typenames () const
      {
        return _typenames;
      }
      void function_type::insert_typename (const std::string& tn) {
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
        boost::apply_visitor (visitor_append_expressions (es), f);
      }

      // ***************************************************************** //

      boost::optional<const id::ref::port&>
      function_type::get_port_in (const std::string & name) const
      {
        return in().get (name);
      }

      boost::optional<const id::ref::port&>
      function_type::get_port_out (const std::string & name) const
      {
        return out().get (name);
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
        return tunnel().get (name);
      }

      // ***************************************************************** //

      std::string function_type::condition (void) const
      {
        return cond.empty()
          ? "true"
          : fhg::util::join (cond.begin(), cond.end(), " && ", "(", ")")
          ;
      }

      // ***************************************************************** //

      void function_type::push ( const id::ref::port & id
                               , ports_type & ports
                               , const ports_type & others
                               , const std::string& descr
                               )
      {
        const id::ref::port& id_old (ports.push (id));

        if (not (id_old == id))
        {
          throw error::duplicate_port (descr, id.get().name(), path);
        }

        const port_type& port (id.get());

        boost::optional<const id::ref::port&> id_other
          (others.get (port.name()));

        if (id_other)
          {
            const port_type& other (id_other->get());

            if (port.type != other.type)
              {
                throw error::port_type_mismatch ( port.name()
                                                , port.type
                                                , other.type
                                                , path
                                                );
              }
          }
      }

      void function_type::push_in (const id::ref::port& id)
      {
        push (id, _in, _out, "in");
      }
      void function_type::push_out (const id::ref::port& id)
      {
        push (id, _out, _in, "out");
      }
      void function_type::push_inout (const id::ref::port& port_id)
      {
        push_in (port_id);
        push_out (port_id.get().clone (id()));
      }
      void function_type::push_tunnel (const id::ref::port& id)
      {
        const id::ref::port& id_old (_tunnel.push (id));

        if (not (id_old == id))
        {
          throw error::duplicate_port ("tunnel", id.get().name(), path);
        }
      }

      // ***************************************************************** //

      xml::parse::structure_type::forbidden_type
        function_type::forbidden_below (void) const
      {
        xml::parse::structure_type::forbidden_type forbidden;

        BOOST_FOREACH (const port_type& port, in().values())
        {
          forbidden.insert (std::make_pair (port.type, port.name()));
        }
        BOOST_FOREACH (const port_type& port, out().values())
        {
          forbidden.insert (std::make_pair (port.type, port.name()));
        }
        BOOST_FOREACH (const port_type& port, tunnel().values())
        {
          forbidden.insert (std::make_pair (port.type, port.name()));
        }

        return forbidden;
      }

      // ***************************************************************** //

      void function_type::resolve ( const state::type & state
                                  , const xml::parse::structure_type::forbidden_type & forbidden
                                  )
      {
        const xml::parse::structure_type::set_type empty;

        resolve (empty, state, forbidden);
      }

      namespace
      {
        class function_resolve : public boost::static_visitor<void>
        {
        private:
          const xml::parse::structure_type::set_type global;
          const state::type & state;
          const xml::parse::structure_type::forbidden_type & forbidden;

        public:
          function_resolve
            ( const xml::parse::structure_type::set_type & _global
            , const state::type & _state
            , const xml::parse::structure_type::forbidden_type & _forbidden
            )
              : global (_global)
              , state (_state)
              , forbidden (_forbidden)
          {}

          void operator () (id::ref::expression &) const { return; }
          void operator () (id::ref::module &) const { return; }
          void operator () (id::ref::net & id) const
          {
            id.get_ref().resolve (global, state, forbidden);
          }
        };
      }

      void function_type::resolve
        ( const xml::parse::structure_type::set_type & global
        , const state::type & state
        , const xml::parse::structure_type::forbidden_type & forbidden
        )
      {
        namespace st = xml::parse::structure_type;

        structs_resolved =
          st::join (global, st::make (structs), forbidden, state);

        for ( st::set_type::iterator pos (structs_resolved.begin())
            ; pos != structs_resolved.end()
            ; ++pos
            )
        {
          boost::apply_visitor
            ( st::resolve (structs_resolved, pos->second.path())
            , pos->second.signature()
            );
        }

        boost::apply_visitor
          (function_resolve ( structs_resolved
                            , state
                            , forbidden_below()
                            )
          , f
          );
      }

      // ***************************************************************** //

      signature::type function_type::type_of_port ( const we::type::PortDirection & dir
                                                  , const port_type & port
                                                  ) const
      {
        if (literal::valid_name (port.type))
        {
          return signature::type (port.type);
        }

        xml::parse::structure_type::set_type::const_iterator sig
          (structs_resolved.find (port.type));

        if (sig == structs_resolved.end())
        {
          throw error::port_with_unknown_type
            (dir, port.name(), port.type, path);
        }

        return signature::type (sig->second.signature(), sig->second.name());
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
        boost::apply_visitor (function_sanity_check (state), f);
      }

      // ***************************************************************** //

      namespace
      {
        class function_type_check : public boost::static_visitor<void>
        {
        private:
          const state::type & state;

        public:
          function_type_check (const state::type & _state) : state (_state) {}

          void operator () (const id::ref::expression &) const { return; }
          void operator () (const id::ref::module &) const { return; }
          void operator () (const id::ref::net& id) const
          {
            id.get().type_check (state);
          }
        };
      }

      void function_type::type_check (const state::type & state) const
      {
        BOOST_FOREACH (const port_type& port, in().values())
        {
          port.type_check ("in", path, state);
        }
        BOOST_FOREACH (const port_type& port, out().values())
        {
          port.type_check ("out", path, state);
        }
        BOOST_FOREACH (const port_type& port, tunnel().values())
        {
          port.type_check ("tunnel", path, state);
        }

        boost::apply_visitor (function_type_check (state), f);
      }

      // ***************************************************************** //

      class function_synthesize
        : public boost::static_visitor<we::activity_t::transition_type>
      {
      private:
        const state::type & state;
        function_type & fun;

        typedef we::activity_t::transition_type we_transition_type;

        typedef we_transition_type::edge_type we_edge_type;

        typedef we_transition_type::expr_type we_expr_type;
        typedef we_transition_type::net_type we_net_type;
        typedef we_transition_type::mod_type we_module_type;
        typedef we_transition_type::preparsed_cond_type we_cond_type;

        typedef we_transition_type::requirement_t we_requirement_type;

        typedef we_transition_type::pid_t pid_t;

        typedef boost::unordered_map<std::string, pid_t> pid_of_place_type;

        void add_ports ( we_transition_type & trans
                       , const function_type::ports_type& ports
                       , const we::type::PortDirection & direction
                       ) const
        {
          BOOST_FOREACH (const port_type& port, ports.values())
            {
              const signature::type type
                (fun.type_of_port (direction, port));

              trans.add_port ( port.name()
                             , type
                             , direction
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
                       , const we::type::PortDirection & direction
                       , const Map & pid_of_place
                       ) const
        {
          BOOST_FOREACH (const port_type& port, ports.values())
            {
              const signature::type type
                (fun.type_of_port (direction, port));

              if (not port.place)
                {
                  trans.add_port ( port.name()
                                 , type
                                 , direction
                                 , port.properties()
                                 );
                }
              else
                {
                  // basically safe, since type checking has verified
                  // the existence and type safety of the place to
                  // connect to

                  trans.add_port ( port.name()
                                 , type
                                 , direction
                                 , get_pid (pid_of_place, *port.place)
                                 , port.properties()
                                 )
                    ;
                }
            }
        }

        void add_requirements ( we_transition_type & trans
                              , xml::parse::type::requirements_type const & req
                              ) const
        {
          for ( requirements_type::const_iterator r (req.begin())
              ; r != req.end()
              ; ++r
              )
          {
            trans.add_requirement (we_requirement_type ( r->first
                                                       , r->second
                                                       )
                                  );
          }
        }

        std::string name (void) const
        {
          if (not fun.name())
            {
              throw error::synthesize_anonymous_function (fun.path);
            }

          return *fun.name();
        }

        we_cond_type condition (void) const
        {
          const std::string cond (fun.condition());

          util::we_parser_t parsed_condition
            (util::we_parse (cond, "condition", "function", name(), fun.path));

          return we_cond_type (cond, parsed_condition);
        }

      public:
        function_synthesize (const state::type & _state, function_type & _fun)
          : state (_state)
          , fun (_fun)
        {}

        we_transition_type
        operator () (const id::ref::expression& id_expression) const
        {
          const std::string expr (id_expression.get().expression());
          const util::we_parser_t parsed_expression
            (util::we_parse (expr, "expression", "function", name(), fun.path));

          we_transition_type trans
            ( name()
            , we_expr_type (expr, parsed_expression)
            , condition()
            , fun.internal.get_value_or (true)
            , fun.properties()
            );

          add_ports (trans, fun.in(), we::type::PORT_IN);
          add_ports (trans, fun.out(), we::type::PORT_OUT);
          add_ports (trans, fun.tunnel(), we::type::PORT_TUNNEL);
          add_requirements (trans, fun.requirements);

          return trans;
        }

        we_transition_type operator () (const id::ref::module& id_mod) const
        {
          const module_type& mod (id_mod.get());

          we_transition_type trans
            ( name()
            , we_module_type (mod.name, mod.function)
            , condition()
            , fun.internal.get_value_or (false)
            , fun.properties()
            );

          add_ports (trans, fun.in(), we::type::PORT_IN);
          add_ports (trans, fun.out(), we::type::PORT_OUT);
          add_ports (trans, fun.tunnel(), we::type::PORT_TUNNEL);
          add_requirements (trans, fun.requirements);

          return trans;
        }

        we_transition_type operator () (const id::ref::net & id_net) const
        {
          we_net_type we_net;

          we::activity_t::transition_type::edge_type e (0);

          pid_of_place_type pid_of_place
            ( net_synthesize ( we_net
                             , place_map_map_type()
                             , id_net.get()
                             , state
                             , e
                             )
            );

          util::property::join ( state
                               , fun.properties()
                               , id_net.get().properties()
                               );

          we_transition_type trans
            ( name()
            , we_net
            , condition()
            , fun.internal.get_value_or (true)
            , fun.properties()
            );

          add_ports (trans, fun.in(), we::type::PORT_IN, pid_of_place);
          add_ports (trans, fun.out(), we::type::PORT_OUT, pid_of_place);
          add_ports (trans, fun.tunnel(), we::type::PORT_TUNNEL, pid_of_place);
          add_requirements (trans, fun.requirements);

          return trans;
        }
      };

      we::activity_t::transition_type
      function_type::synthesize (const state::type & state)
      {
        return boost::apply_visitor
          (function_synthesize (state, *this), f);
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
        BOOST_FOREACH (port_type& port, in().values())
        {
          port.specialize (map, state);
        }
        BOOST_FOREACH (port_type& port, out().values())
        {
          port.specialize (map, state);
        }
        BOOST_FOREACH (port_type& port, tunnel().values())
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
          , f
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
          : public boost::static_visitor<function_type::type>
        {
        public:
          visitor_clone ( const id::function& new_id
                        , id::mapper* const mapper
                        )
            : _new_id (new_id)
            , _mapper (mapper)
          { }
          template<typename ID_TYPE>
            function_type::type operator() (const ID_TYPE& id) const
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
          , _name
          , _in.clone (new_id, new_mapper)
          , _out.clone (new_id, new_mapper)
          , _tunnel.clone (new_id, new_mapper)
          , _typenames
          , contains_a_module_call
          , internal
          , structs
          , cond
          , requirements
          , boost::apply_visitor (visitor_clone (new_id, new_mapper), f)
          , structs_resolved
          , _properties
          , path
          ).make_reference_id();
      }

      // ***************************************************************** //

      fun_info_type::fun_info_type ( const std::string & _name
                                   , const std::string & _code
                                   , const module_type::flags_type & _ldflags
                                   , const module_type::flags_type & _cxxflags
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

            stream.commit();
          }
      }

      void mk_makefile ( const state::type & state
                       , const fun_info_map & m
                       )
      {
        namespace cpp_util = ::fhg::util::cpp;

        const path_t prefix (state.path_to_cpp());
        const path_t file (prefix / "Makefile");

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
        stream << "ifeq \"$(CXX)\" \"\""                           << std::endl;
        stream << "  $(error Variable CXX is empty.)"              << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "CXXFLAGS += -I."                                << std::endl;
        stream << "CXXFLAGS += -I$(BOOST_ROOT)/include"            << std::endl;
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

        BOOST_FOREACH (std::string const& flag, state.gen_cxxflags())
          {
            stream << "CXXFLAGS += " << flag << std::endl;
          }

        stream                                                     << std::endl;

        BOOST_FOREACH (std::string const& flag, state.gen_ldflags())
          {
            stream << "LDFLAGS += " << flag << std::endl;
          }

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
            const std::string ldflags ("LDFLAG_" + mod->first);

            for ( fun_infos_type::const_iterator fun (funs.begin())
                ; fun != funs.end()
                ; ++fun
                )
              {
                stream << "##"                                     << std::endl;
                stream << "## function " << fun->name              << std::endl;
                stream << "##"                                     << std::endl;

                const std::string cxxflags
                  ("CXXFLAG_" + mod->first + "_" + fun->name);

                stream << objs << " += "
                       << cpp_util::make::obj (mod->first, fun->name)
                                                                   << std::endl;

                BOOST_FOREACH (module_type::links_type::value_type const& link, fun->links)
                  {
                    stream << objs << " += "
                           << boost::filesystem::absolute
                              ( link
                              , fun->path.parent_path()
                              ).string()
                           << std::endl;
                  }

                BOOST_FOREACH (module_type::flags_type::value_type const& flag, fun->ldflags)
                  {
                    stream << ldflags << " += " << flag << std::endl;
                  }

                BOOST_FOREACH (module_type::flags_type::value_type const& flag, fun->cxxflags)
                  {
                    stream << cxxflags << " += " << flag << std::endl;
                  }

                stream << cpp_util::make::obj (mod->first, fun->name)
                       << ": "
                       << cpp_util::make::cpp (mod->first, fun->name)
                                                                   << std::endl;
                stream << "\t$(CXX) $(CXXFLAGS)" << " $(" << cxxflags << ")"
                       << " -c $< -o $@"                           << std::endl;
                stream << cpp_util::make::dep (mod->first, fun->name)
                       << ": "
                       << cpp_util::make::cpp (mod->first, fun->name)
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
                   << obj_cpp                                      << std::endl;
            stream << "\t$(CXX) $(CXXFLAGS) -c $< -o $@"           << std::endl;
            stream << cpp_util::make::dep (mod->first)
                   << ": "
                   << obj_cpp                                      << std::endl;
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
            stream << cpp_util::make::mod_so (mod->first)
                   << ": $(" << objs << ")";

            stream << std::endl;

            stream << "\t$(CXX)"
                   << " -shared $^ -o $@"
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

        stream.commit();
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
                  (u.get().name(), trans.name(), trans.path);
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
            << "    namespace " << mod.name << std::endl
            << "    {" << std::endl
            ;
        }

        template<typename Stream>
        void namespace_close (Stream& s, const module_type & mod)
        {
          s << "    } // namespace " << mod.name << std::endl
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
              << mod.function
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
                            , cpp_util::path::op() / mod.name / file_hpp
                            );

          namespace_open (s, mod);

          s << "      "
            << "static void " << mod.function
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

              s << "::we::loader::put (_pnetc_output"
                << ", \"" << (*port_return).name << "\""
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
                                      , mod.name
                                      , mod.function
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
                << "  ::we::loader::put (_pnetc_output"
                << ", \"" << port->name << "\""
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
                << "  ::we::loader::put (_pnetc_output"
                << ", \"" << port->name << "\""
                << ", " << mk_value (*port)
                << ")"
                << ";"
                << std::endl
                ;
            }

          s << "      "
            << "} // " << mod.function << std::endl;

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

            const mcs_type::const_iterator old_map (mcs.find (mod.name));

            if (old_map != mcs.end())
            {
              const mc_by_function_type::const_iterator old_mc
                (old_map->second.find (mod.function));

              if (old_mc != old_map->second.end())
              {
                if (old_mc->second == mod)
                {
                  state.warn ( warning::duplicate_external_function
                               ( mod.function
                               , mod.name
                               , old_mc->second.path
                               , mod.path
                               )
                             );
                }
                else
                {
                  throw error::duplicate_external_function
                    ( mod.function
                    , mod.name
                    , old_mc->second.path
                    , mod.path
                    );
                }
              }
            }

            mcs[mod.name].insert (std::make_pair (mod.function, mod));

            ports_with_type_type ports_const;
            ports_with_type_type ports_mutable;
            ports_with_type_type ports_out;
            boost::optional<port_with_type> port_return;
            types_type types;

            if (mod.port_return)
            {
              boost::optional<const id::ref::port&> id_port
                (_id_function.get().get_port_out (*mod.port_return));

              const port_type& port (id_port->get());

              port_return = port_with_type (*mod.port_return, port.type);
              types.insert (port.type);
            }

            for ( module_type::port_args_type::const_iterator name (mod.port_arg.begin())
                ; name != mod.port_arg.end()
                ; ++name
                )
            {
              if (_id_function.get().is_known_port_inout (*name))
              {
                boost::optional<const id::ref::port&> id_port_in
                  (_id_function.get().get_port_in (*name));
                boost::optional<const id::ref::port&> id_port_out
                  (_id_function.get().get_port_out (*name));

                const port_type& port_in (id_port_in->get());
                const port_type& port_out (id_port_out->get());

                if (    mod.port_return
                   && (*mod.port_return == port_in.name())
                   )
                {
                  ports_const.push_back (port_with_type (*name, port_in.type));
                  types.insert (port_in.type);
                }
                else
                {
                  ports_mutable.push_back (port_with_type (*name, port_in.type));
                  types.insert (port_in.type);
                }
              }
              else if (_id_function.get().is_known_port_in (*name))
              {
                boost::optional<const id::ref::port&> id_port_in
                  (_id_function.get().get_port_in (*name));

                const port_type& port_in (id_port_in->get());

                ports_const.push_back (port_with_type (*name, port_in.type));
                types.insert (port_in.type);
              }
              else if (_id_function.get().is_known_port_out (*name))
              {
                boost::optional<const id::ref::port&> id_port_out
                  (_id_function.get().get_port_out (*name));

                const port_type& port_out (id_port_out->get());

                if (    mod.port_return
                   && (*mod.port_return == port_out.name())
                   )
                {
                  // do nothing, it is the return port
                }
                else
                {
                  ports_out.push_back (port_with_type (*name, port_out.type));
                  types.insert (port_out.type);
                }
              }
            }

            const path_t prefix (state.path_to_cpp());
            const path_t path (prefix / cpp_util::path::op() / mod.name);
            const std::string file_hpp (cpp_util::make::hpp (mod.function));
            const std::string file_cpp
              ( mod.code
              ? cpp_util::make::cpp (mod.function)
              : cpp_util::make::tmpl (mod.function)
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

              const fun_info_type fun_info ( mod.function
                                           , stream.str()
                                           , mod.ldflags
                                           , mod.cxxflags
                                           , mod.links
                                           , mod.path
                                           );

              m[mod.name].insert (fun_info);
            }

            {
              const path_t file (path / file_hpp);

              util::check_no_change_fstream stream (state, file);

              cpp_util::header_gen_full (stream);
              cpp_util::include_guard_begin
                (stream, "PNETC_OP_" + mod.name + "_" + mod.function);

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
                (stream, "PNETC_OP_" + mod.name + "_" + mod.function);

              stream.commit();
            }

            {
              const path_t file (path / file_cpp);

              util::check_no_change_fstream stream (state, file);

              cpp_util::header_gen (stream);

              cpp_util::include ( stream
                                , cpp_util::path::op() / mod.name / file_hpp
                                );

              for ( module_type::cincludes_type::const_iterator inc
                      (mod.cincludes.begin())
                  ; inc != mod.cincludes.end()
                  ; ++inc
                  )
              {
                cpp_util::include (stream, *inc);
              }

              if (not mod.code)
              {
                cpp_util::include (stream, "stdexcept");
              }

              namespace_open (stream, mod);

              mod_signature ( stream
                            , port_return
                            , ports_const, ports_mutable, ports_out, mod
                            );

              stream << std::endl << "      {" << std::endl;

              if (not mod.code)
              {
                stream << "        // INSERT CODE HERE" << std::endl
                       << "        throw std::runtime_error (\""
                       << mod.name << "::" << mod.function
                       << ": NOT YET IMPLEMENTED\");";
              }
              else
              {
                stream << *mod.code;
              }

              stream << std::endl << "      }" << std::endl;

              namespace_close (stream, mod);

              stream.commit();
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
          , id_function.get_ref().f
          );

        return id_function.get().contains_a_module_call;
      }

      // **************************************************************** //

      namespace
      {
        void to_cpp (const structure_type& s, const state::type & state)
        {
          namespace cpp_util = ::fhg::util::cpp;

          typedef boost::filesystem::path path_t;

          const path_t prefix (state.path_to_cpp());
          const path_t file
            (prefix / cpp_util::path::type() / cpp_util::make::hpp (s.name()));

          util::check_no_change_fstream stream (state, file);

          signature::cpp::cpp_header
            (stream, s.signature(), s.name(), s.path(), cpp_util::path::type());

          stream.commit();
        }

        void to_cpp (const structs_type& structs, const state::type& state)
        {
          BOOST_FOREACH (const structure_type& structure, structs)
          {
            to_cpp (structure, state);
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

          boost::apply_visitor (visitor_to_cpp (state), function.f);
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

        void dump_before_property ( ::fhg::util::xml::xmlstream & s
                                  , const function_type & f
                                  )
        {
          s.open ("defun");
          s.attr ("name", f.name());
          s.attr ("internal", f.internal);
        }

        void dump_after_property ( ::fhg::util::xml::xmlstream & s
                                 , const function_type & f
                                 )
        {
          BOOST_FOREACH (const std::string& tn, f.typenames())
            {
              s.open ("template-parameter");
              s.attr ("type", tn);
              s.close();
            }

          dumps (s, f.structs.begin(), f.structs.end());

          xml::parse::type::dump::dump (s, f.requirements);

          dumps (s, f.in().values(), "in");
          dumps (s, f.out().values(), "out");
          dumps (s, f.tunnel().values(), "tunnel");

          boost::apply_visitor (function_dump_visitor (s), f.f);

          for ( conditions_type::const_iterator cond (f.cond.begin())
              ; cond != f.cond.end()
              ; ++cond
              )
            {
              s.open ("condition");
              s.content (*cond);
              s.close ();
            }

          s.close ();
        }

        void dump ( ::fhg::util::xml::xmlstream & s
                  , const function_type & f
                  , const state::type & state
                  )
        {
          dump_before_property (s, f);

          state.dump_context (s);

          ::we::type::property::dump::dump (s, f.properties());

          dump_after_property (s, f);
        }

        void dump ( ::fhg::util::xml::xmlstream & s
                  , const function_type & f
                  )
        {
          dump_before_property (s, f);

          ::we::type::property::dump::dump (s, f.properties());

          dump_after_property (s, f);
        }
      } // namespace dump
    }
  }
}
