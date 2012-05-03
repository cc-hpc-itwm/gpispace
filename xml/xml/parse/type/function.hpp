// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_FUNCTION_HPP
#define _XML_PARSE_TYPE_FUNCTION_HPP

#include <xml/parse/types.hpp>

#include <xml/parse/util/unique.hpp>
#include <xml/parse/util/weparse.hpp>
#include <xml/parse/util/property.hpp>
#include <xml/parse/util/mk_fstream.hpp>

#include <list>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/variant.hpp>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <we/type/literal/valid_name.hpp>
#include <we/type/property.hpp>

#include <we/we.hpp>
#include <we/type/signature/cpp.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/maybe.hpp>

#include <fhg/util/filesystem.hpp>
#include <fhg/util/cpp.hpp>

#include <fhg/util/xml.hpp>

#include <fstream>

namespace cpp_util = ::fhg::util::cpp;
namespace xml_util = ::fhg::util::xml;

#include <iostream>
#include <sstream>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef xml::util::unique<port_type>::elements_type ports_type;
      typedef std::list<std::string> conditions_type;
      typedef xml::util::unique<function_type>::elements_type functions_type;

      // ******************************************************************* //

      template<typename Net>
      class function_resolve : public boost::static_visitor<void>
      {
      private:
        const xml::parse::struct_t::set_type global;
        const state::type & state;
        const xml::parse::struct_t::forbidden_type & forbidden;

      public:
        function_resolve
        ( const xml::parse::struct_t::set_type & _global
        , const state::type & _state
        , const xml::parse::struct_t::forbidden_type & _forbidden
        )
          : global (_global)
          , state (_state)
          , forbidden (_forbidden)
        {}

        void operator () (expression_type &) const { return; }
        void operator () (mod_type &) const { return; }
        void operator () (Net & net) const
        {
          net.resolve (global, state, forbidden);
        }
      };

      // ******************************************************************* //

      template<typename Net, typename Fun>
      class function_specialize : public boost::static_visitor<void>
      {
      private:
        const type::type_map_type & map;
        const type::type_get_type & get;
        const xml::parse::struct_t::set_type & known_structs;
        const state::type & state;
        Fun & fun;

      public:
        function_specialize
        ( const type::type_map_type & _map
        , const type::type_get_type & _get
        , const xml::parse::struct_t::set_type & _known_structs
        , const state::type & _state
        , Fun & _fun
        )
          : map (_map)
          , get (_get)
          , known_structs (_known_structs)
          , state (_state)
          , fun (_fun)
        {}

        void operator () (expression_type &) const { return; }
        void operator () (mod_type &) const { return; }
        void operator () (Net & net) const
        {
          net.specialize (map, get, known_structs, state);

          split_structs ( known_structs
                        , net.structs
                        , fun.structs
                        , get
                        , state
                        );
        }
      };

      // ******************************************************************* //

      template<typename Net, typename Fun>
      class function_sanity_check : public boost::static_visitor<void>
      {
      private:
        const state::type & state;
        const Fun & fun;

      public:
        function_sanity_check ( const state::type & _state
                              , const Fun & _fun
                              )
          : state (_state)
          , fun (_fun)
        {}

        void operator () (const expression_type &) const { return; }
        void operator () (const mod_type & m) const { m.sanity_check (fun); }
        void operator () (const Net & net) const { net.sanity_check (state); }
      };

      // ******************************************************************* //

      template<typename Net>
      class function_type_check : public boost::static_visitor<void>
      {
      private:
        const state::type & state;

      public:
        function_type_check (const state::type & _state) : state (_state) {}

        void operator () (const expression_type &) const { return; }
        void operator () (const mod_type &) const { return; }
        void operator () (const Net & net) const { net.type_check (state); }
      };

      // ******************************************************************* //

      template<typename Net>
      class function_distribute_function : public boost::static_visitor<void>
      {
      private:
        const state::type& _state;
        const functions_type& _functions;

      public:
        function_distribute_function ( const state::type& state
                                     , const functions_type& functions
                                     )
          : _state (state)
          , _functions (functions)
        {}

        void operator () (expression_type&) const { return; }
        void operator () (mod_type&) const { return; }
        void operator () (Net& net) const
        {
          net.distribute_function (_state, _functions);
        }
      };

      // ******************************************************************* //

      template< typename Activity
              , typename Net
              , typename Trans
              , typename Fun
              , typename Map
              >
      void
      transition_synthesize ( const Trans &
                            , const state::type &
                            , const Net &
                            , typename Activity::transition_type::net_type &
                            , const Map &
                            , typename Activity::transition_type::edge_type &
                            );

      template <typename Activity, typename Net, typename Fun>
      boost::unordered_map< std::string
                          , typename Activity::transition_type::pid_t
                          >
      net_synthesize ( typename Activity::transition_type::net_type &
                     , const place_map_map_type &
                     , const Net &
                     , const state::type &
                     , typename Activity::transition_type::edge_type &
                     );

      // ******************************************************************* //

      template<typename Map>
      typename Map::mapped_type
      get_pid (const Map & pid_of_place, const std::string name)
      {
        const typename Map::const_iterator pos (pid_of_place.find (name));

        if (pos == pid_of_place.end())
          {
            THROW_STRANGE ("missing place " << name << " in pid_of_place");
          }

        return pos->second;
      };

      // ******************************************************************* //

      template<typename Activity, typename Net, typename Fun>
      class function_synthesize
        : public boost::static_visitor<typename Activity::transition_type>
      {
      private:
        const state::type & state;
        Fun & fun;

        typedef typename Activity::transition_type we_transition_type;

        typedef typename we_transition_type::place_type we_place_type;
        typedef typename we_transition_type::edge_type we_edge_type;

        typedef typename we_transition_type::expr_type we_expr_type;
        typedef typename we_transition_type::net_type we_net_type;
        typedef typename we_transition_type::mod_type we_mod_type;
        typedef typename we_transition_type::preparsed_cond_type we_cond_type;

        typedef typename we_transition_type::requirement_t we_requirement_type;

        typedef typename we_transition_type::pid_t pid_t;

        typedef boost::unordered_map<std::string, pid_t> pid_of_place_type;

        void add_ports ( we_transition_type & trans
                       , const ports_type & ports
                       , const we::type::PortDirection & direction
                       ) const
        {
          for ( ports_type::const_iterator port (ports.begin())
              ; port != ports.end()
              ; ++port
              )
            {
              const signature::type type
                (fun.type_of_port (direction, *port));

              trans.add_ports () (port->name, type, direction, port->prop);
            }
        }

        template<typename Map>
        void add_ports ( we_transition_type & trans
                       , const ports_type & ports
                       , const we::type::PortDirection & direction
                       , const Map & pid_of_place
                       ) const
        {
          for ( ports_type::const_iterator port (ports.begin())
              ; port != ports.end()
              ; ++port
              )
            {
              const signature::type type
                (fun.type_of_port (direction, *port));

              if (port->place.isNothing())
                {
                  trans.add_ports () (port->name, type, direction, port->prop);
                }
              else
                {
                  // basically safe, since type checking has verified
                  // the existence and type safety of the place to
                  // connect to

                  trans.add_ports () ( port->name
                                     , type
                                     , direction
                                     , get_pid (pid_of_place, *port->place)
                                     , port->prop
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
          if (fun.name.isNothing())
            {
              throw error::synthesize_anonymous_function (fun.path);
            }

          return *fun.name;
        }

        we_cond_type condition (void) const
        {
          const std::string cond (fun.condition());

          util::we_parser_t parsed_condition
            (util::we_parse (cond, "condition", "function", name(), fun.path));

          return we_cond_type (cond, parsed_condition);
        }

      public:
        function_synthesize (const state::type & _state, Fun & _fun)
          : state (_state)
          , fun (_fun)
        {}

        we_transition_type operator () (const expression_type & e) const
        {
          const std::string expr (e.expression());
          const util::we_parser_t parsed_expression
            (util::we_parse (expr, "expression", "function", name(), fun.path));

          we_transition_type trans
            ( name()
            , we_expr_type (expr, parsed_expression)
            , condition()
            , fun.internal.get_with_default (true)
            , fun.prop
            );

          add_ports (trans, fun.in(), we::type::PORT_IN);
          add_ports (trans, fun.out(), we::type::PORT_OUT);
          add_requirements (trans, fun.requirements);

          return trans;
        }

        we_transition_type operator () (const mod_type & mod) const
        {
          we_transition_type trans
            ( name()
            , we_mod_type (mod.name, mod.function)
            , condition()
            , fun.internal.get_with_default (false)
            , fun.prop
            );

          add_ports (trans, fun.in(), we::type::PORT_IN);
          add_ports (trans, fun.out(), we::type::PORT_OUT);
          add_requirements (trans, fun.requirements);

          return trans;
        }

        we_transition_type operator () (const Net & net) const
        {
          we_net_type we_net;

          typename Activity::transition_type::edge_type e (0);

          pid_of_place_type pid_of_place
            ( net_synthesize<Activity, Net, Fun>
              ( we_net
              , place_map_map_type()
              , net
              , state
              , e
              )
            );

          util::property::join (state, fun.prop, net.prop);

          we_transition_type trans
            ( name()
            , we_net
            , condition()
            , fun.internal.get_with_default (true)
            , fun.prop
            );

          add_ports (trans, fun.in(), we::type::PORT_IN, pid_of_place);
          add_ports (trans, fun.out(), we::type::PORT_OUT, pid_of_place);
          add_requirements (trans, fun.requirements);

          return trans;
        }
      };

      // ******************************************************************* //

      class function_is_net : public boost::static_visitor<bool>
      {
      public:
        bool operator () (const net_type &) const { return true; }
        template<typename T>
        bool operator () (const T &) const { return false; }
      };

      // ******************************************************************* //

      struct function_type
      {
      private:
        typedef xml::util::unique<port_type> unique_port_type;

        unique_port_type _in;
        unique_port_type _out;
        unique_port_type _tunnel;

        // ***************************************************************** //

        void push ( const port_type & p
                  , unique_port_type & ports
                  , const unique_port_type & others
                  , const std::string descr
                  )
        {
          port_type old;

          if (!ports.push (p, old))
            {
              throw error::duplicate_port (descr, p.name, path);
            }

          port_type other;

          if (others.by_key (p.name, other) && p.type != other.type)
            {
              throw error::port_type_mismatch ( p.name
                                              , p.type
                                              , other.type
                                              , path
                                              );
            }
        }

        // ***************************************************************** //

      public:
        typedef boost::variant < expression_type
                               , mod_type
                               , boost::recursive_wrapper<net_type>
                               > type;

        bool contains_a_module_call;
        structs_type structs;

        fhg::util::maybe<std::string> name;
        fhg::util::maybe<bool> internal;

        conditions_type cond;

        requirements_type requirements;

        we::type::property::type prop;

        type f;

        boost::filesystem::path path;

        xml::parse::struct_t::set_type structs_resolved;

        bool was_template;

        // ***************************************************************** //

        function_type () {}
        function_type (const type& _f) : f (_f) {}

        // ***************************************************************** //

        void add_expression (const expression_type & e)
        {
          boost::apply_visitor (visitor::join (e), f);
        }

        // ***************************************************************** //

        const ports_type & in (void) const { return _in.elements(); }
        const ports_type & out (void) const { return _out.elements(); }
        const ports_type& tunnel (void) const { return _tunnel.elements(); }

        bool get_port_in (const std::string & name, port_type & port) const
        {
          return _in.by_key (name, port);
        }

        bool get_port_out (const std::string & name, port_type & port) const
        {
          return _out.by_key (name, port);
        }

        bool is_known_port_in (const std::string & name) const
        {
          return _in.is_element (name);
        }

        bool is_known_port_out (const std::string & name) const
        {
          return _out.is_element (name);
        }

        bool is_known_port (const std::string & name) const
        {
          return is_known_port_in (name) || is_known_port_out (name);
        }

        bool is_known_port_inout (const std::string & name) const
        {
          return is_known_port_in (name) && is_known_port_out (name);
        }

        // ***************************************************************** //

        std::string condition (void) const
        {
          return cond.empty()
            ? "true"
            : fhg::util::join (cond.begin(), cond.end(), " && ", "(", ")")
            ;
        }

        // ***************************************************************** //

        void push_in (const port_type & p) { push (p, _in, _out, "in"); }
        void push_out (const port_type & p) { push (p, _out, _in, "out"); }
        void push_inout (const port_type & p) { push_in (p); push_out (p); }
        void push_tunnel (const port_type& p)
        {
          port_type old;

          if (!_tunnel.push (p, old))
            {
              throw error::duplicate_port ("tunnel", p.name, path);
            }
        }

        // ***************************************************************** //

        xml::parse::struct_t::forbidden_type
        forbidden_below (void) const
        {
          xml::parse::struct_t::forbidden_type forbidden;

          for ( ports_type::const_iterator pos (in().begin())
              ; pos != in().end()
              ; ++pos
              )
            {
              forbidden[pos->type] = pos->name;
            }

          for ( ports_type::const_iterator pos (out().begin())
              ; pos != out().end()
              ; ++pos
              )
            {
              forbidden[pos->type] = pos->name;
            }

          return forbidden;
        }

        // ***************************************************************** //

        void distribute_function (const state::type& state)
        {
          distribute_function (state, functions_type());
        }

        void distribute_function ( const state::type& state
                                 , const functions_type& functions
                                 )
        {
          boost::apply_visitor
            ( function_distribute_function<net_type> (state, functions)
            , f
            );
        }

        // ***************************************************************** //

        void resolve ( const state::type & state
                     , const xml::parse::struct_t::forbidden_type & forbidden
                     )
        {
          const xml::parse::struct_t::set_type empty;

          resolve (empty, state, forbidden);
        }

        void resolve
        ( const xml::parse::struct_t::set_type & global
        , const state::type & state
        , const xml::parse::struct_t::forbidden_type & forbidden
        )
        {
          namespace st = xml::parse::struct_t;

          structs_resolved =
            st::join (global, st::make (structs), forbidden, state);

          for ( st::set_type::iterator pos (structs_resolved.begin())
              ; pos != structs_resolved.end()
              ; ++pos
              )
            {
              boost::apply_visitor
                ( st::resolve (structs_resolved, pos->second.path)
                , pos->second.sig
                );
            }

          boost::apply_visitor
            (function_resolve<net_type> ( structs_resolved
                                        , state
                                        , forbidden_below()
                                        )
            , f
            );
        }

        // ***************************************************************** //

        signature::type type_of_port ( const we::type::PortDirection & dir
                                     , const port_type & port
                                     ) const
        {
          if (literal::valid_name (port.type))
            {
              return signature::type (port.type);
            }

          xml::parse::struct_t::set_type::const_iterator sig
            (structs_resolved.find (port.type));

          if (sig == structs_resolved.end())
            {
              throw error::port_with_unknown_type
                (dir, port.name, port.type, path);
            }

          return signature::type (sig->second.sig, sig->second.name);
        };

        // ***************************************************************** //

        void sanity_check (const state::type & state) const
        {
          boost::apply_visitor
            ( function_sanity_check<net_type, function_type> (state, *this)
            , f
            );
        }

        // ***************************************************************** //

        void type_check (const state::type & state) const
        {
          for ( ports_type::const_iterator port (in().begin())
              ; port != in().end()
              ; ++port
              )
            {
              boost::apply_visitor
                (port_type_check<net_type> ("in", *port, path, state), f);
            }

          for ( ports_type::const_iterator port (out().begin())
              ; port != out().end()
              ; ++port
              )
            {
              boost::apply_visitor
                (port_type_check<net_type> ("out", *port, path, state), f);
            }

          boost::apply_visitor (function_type_check<net_type> (state), f);
        }

        // ***************************************************************** //

        template<typename Activity>
        typename Activity::transition_type
        synthesize (const state::type & state)
        {
          return boost::apply_visitor
            ( function_synthesize< Activity
                                 , net_type
                                 , function_type
                                 > (state, *this)
            , f
            );
        }

        // ***************************************************************** //

        void specialize (const state::type & state)
        {
          if (was_template)
            {
              return;
            }

          const type_map_type type_map_empty;
          const type_get_type type_get_empty;
          const xml::parse::struct_t::set_type known_empty;

          specialize ( type_map_empty
                     , type_get_empty
                     , known_empty
                     , state
                     );
        }

        void specialize ( const type_map_type & map
                        , const type_get_type & get
                        , const xml::parse::struct_t::set_type & known_structs
                        , const state::type & state
                        )
        {
          if (was_template)
            {
              return;
            }

          for ( ports_type::iterator port (_in.elements().begin())
              ; port != _in.elements().end()
              ; ++port
              )
            {
              port->specialize (map,  state);
            }

          for ( ports_type::iterator port (_out.elements().begin())
              ; port != _out.elements().end()
              ; ++port
              )
            {
              port->specialize (map, state);
            }

          specialize_structs (map, structs, state);

          namespace st = xml::parse::struct_t;

          boost::apply_visitor
            ( function_specialize<net_type, function_type>
              ( map
              , get
              , st::join (known_structs, st::make (structs), state)
              , state
              , *this
              )
            , f
            );
        }
      };

      // ***************************************************************** //

      struct fun_info_type
      {
        std::string name;
        std::string code;
        flags_type ldflags;
        flags_type cxxflags;

        fun_info_type ( const std::string & _name
                      , const std::string & _code
                      , const flags_type & _ldflags
                      , const flags_type & _cxxflags
                      )
          : name (_name)
          , code (_code)
          , ldflags (_ldflags)
          , cxxflags (_cxxflags)
        {}

        bool operator == (const fun_info_type & other) const
        {
          return name == other.name;
        }

        friend std::size_t hash_value (const fun_info_type &);
      };

      inline std::size_t hash_value (const fun_info_type & fi)
      {
        boost::hash<std::string> hasher;
        return hasher (fi.name);
      }

      typedef boost::unordered_set<fun_info_type> fun_infos_type;

      typedef boost::unordered_map<std::string,fun_infos_type> fun_info_map;

      typedef boost::filesystem::path path_t;

      inline void mk_wrapper ( const state::type & state
                             , const fun_info_map & m
                             )
      {
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

      inline void mk_makefile ( const state::type & state
                              , const fun_info_map & m
                              )
      {
        const path_t prefix (state.path_to_cpp());
        const path_t file (prefix / "Makefile");

        util::check_no_change_fstream stream (state, file);

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
        stream << "  BOOST_ROOT = /usr"                            << std::endl;
        stream << "endif"                                          << std::endl;
        stream                                                     << std::endl;
        stream << "CXXFLAGS += -I."                                << std::endl;
        stream << "CXXFLAGS += -I$(BOOST_ROOT)/include"            << std::endl;
        stream                                                     << std::endl;
        stream << "LDFLAGS += -L$(BOOST_ROOT)/lib"                 << std::endl;
        stream << "LDFLAGS += -lboost_serialization"               << std::endl;
        stream                                                     << std::endl;
        stream << "ifndef CP"                                      << std::endl;
        stream << "  CP = cp"                                      << std::endl;
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
        stream << ".PHONY: default modules"                        << std::endl;
        stream                                                     << std::endl;
        stream << "default: $(MODULES)"                            << std::endl;
        stream << "modules: $(MODULES) objcleandep"                << std::endl;
        stream                                                     << std::endl;
        stream << "%.cpp: %.cpp_tmpl"                              << std::endl;
        stream << "\t$(warning !!!)"                               << std::endl;
        stream << "\t$(warning !!! COPY $*.cpp_tmpl TO $*.cpp)"    << std::endl;
        stream << "\t$(warning !!! THIS IS PROBABLY NOT WHAT YOU WANT!)"
                                                                   << std::endl;
        stream << "\t$(warning !!!)"                               << std::endl;
        stream << "\t$(CP) $^ $@"                                  << std::endl;
        stream                                                     << std::endl;

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            const std::string objs ("OBJ_" + mod->first);
            const fun_infos_type & funs (mod->second);
            const std::string ldflags ("LDFLAG_" + mod->first);

            for ( fun_infos_type::const_iterator fun (funs.begin())
                ; fun != funs.end()
                ; ++fun
                )
              {
                const std::string cxxflags
                  ("CXXFLAG_" + mod->first + "_" + fun->name);

                stream << objs << " += "
                       << cpp_util::make::obj (mod->first, fun->name)
                                                                   << std::endl;

                BOOST_FOREACH (flags_type::value_type const& flag, fun->ldflags)
                  {
                    stream << ldflags << " += " << flag << std::endl;
                  }

                BOOST_FOREACH (flags_type::value_type const& flag, fun->cxxflags)
                  {
                    stream << cxxflags << " += " << flag << std::endl;
                  }

                stream << cpp_util::make::obj (mod->first, fun->name)
                       << ": "
                       << cpp_util::make::cpp (mod->first, fun->name)
                                                                   << std::endl;
                stream << "\t$(CXX) $(CXXFLAGS)" << " $(" << cxxflags << ")"
                       << " -c $^ -o $@"                           << std::endl;
                stream                                             << std::endl;
              }

            stream << objs << " += " << cpp_util::make::obj (mod->first)
                                                                   << std::endl;

            stream << cpp_util::make::obj (mod->first)
                   << ": "
                   << ( cpp_util::path::op()
                      / cpp_util::make::cpp (mod->first)
                      ).string()                                   << std::endl;
            stream << "\t$(CXX) $(CXXFLAGS) -c $^ -o $@"           << std::endl;
            stream                                                 << std::endl;

            stream                                                 << std::endl;
            stream << cpp_util::make::mod_so (mod->first)
                   << ": $(" << objs << ")";

            stream << std::endl;

            stream << "\t$(CXX)"
                   << " -shared $^ -o $@"
                   << " $(LDFLAGS)"
                   << " $(" << ldflags << ")"                      << std::endl;
            stream                                                 << std::endl;
          }

        stream << ".PHONY: clean objclean modclean objcleandep"    << std::endl;
        stream                                                     << std::endl;
        stream << "clean: objclean modclean"                       << std::endl;
        stream                                                     << std::endl;
        stream << "objclean:"                                      << std::endl;

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            stream << "\t$(RM) $(OBJ_" << mod->first << ")"        << std::endl;
          }

        stream                                                     << std::endl;
        stream << "objcleandep: $(MODULES)"                        << std::endl;

        for ( fun_info_map::const_iterator mod (m.begin())
            ; mod != m.end()
            ; ++mod
            )
          {
            stream << "\t$(RM) $(OBJ_" << mod->first << ")"        << std::endl;
          }

        stream                                                     << std::endl;
        stream << "modclean:"                                      << std::endl;
        stream << "\t$(RM) $(MODULES)"                             << std::endl;

        stream.commit();
      }

      inline bool find_module_calls ( const state::type &
                                    , function_type &
                                    , fun_info_map &
                                    );

      namespace visitor
      {
        template<typename NET, typename TRANS>
        class transition_find_module_calls : public boost::static_visitor<bool>
        {
        private:
          const state::type & state;
          const NET & net;
          const TRANS & trans;
          fun_info_map & m;

        public:
          transition_find_module_calls ( const state::type & _state
                                       , const NET & _net
                                       , const TRANS & _trans
                                       , fun_info_map & _m
                                       )
            : state (_state)
            , net (_net)
            , trans (_trans)
            , m (_m)
          {}

          bool operator () (function_type & f) const
          {
            return xml::parse::type::find_module_calls (state, f, m);
          }

          bool operator () (use_type & u) const
          {
            function_type fun;

            if (!net.get_function (u.name, fun))
              {
                throw error::unknown_function
                  (u.name, trans.name, trans.path);
              }

            return xml::parse::type::find_module_calls (state, fun, m);
          }
        };
      }

      template<typename NET>
      inline bool find_module_calls ( const state::type & state
                                    , NET & n
                                    , fun_info_map & m
                                    )
      {
        n.contains_a_module_call = false;

        for ( typename NET::transitions_type::iterator pos
                (n.transitions().begin())
            ; pos != n.transitions().end()
            ; ++pos
            )
          {
            n.contains_a_module_call
              |= boost::apply_visitor
              ( visitor::transition_find_module_calls< NET
                                                     , transition_type
                                                     > (state, n, *pos, m)
              , pos->f
              );
          }

        return n.contains_a_module_call;
      }

      namespace visitor
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
          {}
        };

        typedef std::list<port_with_type> ports_type;

        template<typename Stream>
        void mod_includes (Stream& s, const types_type& types)
        {
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
        void namespace_open (Stream& s, const mod_type& mod)
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
        void namespace_close (Stream& s, const mod_type & mod)
        {
          s << "    } // namespace " << mod.name << std::endl
            << "  } // namespace op" << std::endl
            << "} // namespace pnetc" << std::endl
            ;
        }

        inline std::string mk_type (const std::string & type)
        {
          return literal::cpp::known (type)
            ? literal::cpp::translate (type)
            : cpp_util::access::make ("", "pnetc", "type", type, type)
            ;
        }

        inline std::string mk_get ( const port_with_type & port
                                  , const std::string & amper = ""
                                  )
        {
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

        inline std::string mk_value (const port_with_type & port)
        {
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
                      , const fhg::util::maybe<port_with_type> & port_return
                      , const ports_type & ports_const
                      , const ports_type & ports_mutable
                      , const ports_type & ports_out
                      , const mod_type & mod
                      )
        {
          std::ostringstream pre;

          pre << "      "
              << (port_return.isJust() ? mk_type ((*port_return).type) : "void")
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

          for ( ports_type::const_iterator port (ports_const.begin())
              ; port != ports_const.end()
              ; ++port, first = false
              )
            {
              s << (first ? " " : (white + ", "))
                << "const " << mk_type (port->type) << " & " << port->name
                << std::endl
                ;
            }

          for ( ports_type::const_iterator port (ports_mutable.begin())
              ; port != ports_mutable.end()
              ; ++port, first = false
              )
            {
              s << (first ? " " : (white + ", "))
                << mk_type (port->type) << " & " << port->name
                << std::endl
                ;
            }

          for ( ports_type::const_iterator port (ports_out.begin())
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
                    , const mod_type & mod
                    , const path_t file_hpp
                    , const ports_type & ports_const
                    , const ports_type & ports_mutable
                    , const ports_type & ports_out
                    , const fhg::util::maybe<port_with_type> & port_return
                    )
        {
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

          for ( ports_type::const_iterator port (ports_const.begin())
              ; port != ports_const.end()
              ; ++port
              )
            {
              s << "      "
                << "  const " << mk_get (*port, "& ");
            }

          for ( ports_type::const_iterator port (ports_mutable.begin())
              ; port != ports_mutable.end()
              ; ++port
              )
            {
              s << "      "
                << "  " << mk_get (*port);
            }

          for ( ports_type::const_iterator port (ports_out.begin())
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

          if (port_return.isJust())
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

          for ( ports_type::const_iterator port (ports_const.begin())
              ; port != ports_const.end()
              ; ++port, first_param = false
              )
            {
              s << (first_param ? "" : ", ") << port->name;
            }

          for ( ports_type::const_iterator port (ports_mutable.begin())
              ; port != ports_mutable.end()
              ; ++port, first_param = false
              )
            {
              s << (first_param ? "" : ", ") << port->name;
            }

          for ( ports_type::const_iterator port (ports_out.begin())
              ; port != ports_out.end()
              ; ++port, first_param = false
              )
            {
              s << (first_param ? "" : ", ") << port->name;
            }

          s << ")";

          if (port_return.isJust())
            {
              s << ")";

              if (!literal::cpp::known ((*port_return).type))
                {
                  s << ")";
                }
            }

          s << ";" << std::endl;

          for ( ports_type::const_iterator port (ports_mutable.begin())
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

          for ( ports_type::const_iterator port (ports_out.begin())
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

        template<typename NET>
        class find_module_calls : public boost::static_visitor<bool>
        {
        private:
          const state::type & state;
          const function_type & f;
          fun_info_map & m;

        public:
          find_module_calls ( const state::type & _state
                            , const function_type & _f
                            , fun_info_map & _m
                            )
            : state (_state)
            , f (_f)
            , m (_m)
          {}

          bool operator () (expression_type &) const
          {
            return false;
          }

          bool operator () (NET & n) const
          {
            return xml::parse::type::find_module_calls<NET> (state, n, m);
          }

          bool operator () (mod_type & mod) const
          {
#define STRANGE(msg) THROW_STRANGE(  msg << " in module "     \
                                  << mod.name << " function " \
                                  << mod.function             \
                                  )

            ports_type ports_const;
            ports_type ports_mutable;
            ports_type ports_out;
            fhg::util::maybe<port_with_type> port_return;
            types_type types;

            if (mod.port_return.isJust())
              {
                port_type port;

                if (!f.get_port_out (*mod.port_return, port))
                  {
                    STRANGE ("unknown return port " << *mod.port_return);
                  }

                port_return = port_with_type (*mod.port_return, port.type);
                types.insert (port.type);
              }

            for ( port_args_type::const_iterator name (mod.port_arg.begin())
                ; name != mod.port_arg.end()
                ; ++name
                )
              {
                if (!f.is_known_port (*name))
                  {
                    STRANGE ("unknown arg port " << *name);
                  }

                if (f.is_known_port_inout (*name))
                  {
                    port_type port_in;
                    port_type port_out;

                    if (!f.get_port_in (*name, port_in))
                      {
                        STRANGE ("failed to get port_in " << *name);
                      }

                    if (!f.get_port_out (*name, port_out))
                      {
                        STRANGE ("failed to get port_out " << *name);
                      }

                    if (port_in.type != port_out.type)
                      {
                        STRANGE ("in-type " << port_in.type
                                << " != out-type " << port_out.type
                                << " for port_inout " << *name
                                );
                      }

                    if (    mod.port_return.isJust()
                       && (*mod.port_return == port_in.name)
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
                else if (f.is_known_port_in (*name))
                  {
                    port_type port_in;

                    if (!f.get_port_in (*name, port_in))
                      {
                        STRANGE ("failed to get port_in " << *name);
                      }

                    ports_const.push_back (port_with_type (*name, port_in.type));
                    types.insert (port_in.type);
                  }
                else if (f.is_known_port_out (*name))
                  {
                    port_type port_out;

                    if (!f.get_port_out (*name, port_out))
                      {
                        STRANGE ("failed to get port_out " << *name);
                      }

                    if (    mod.port_return.isJust()
                       && (*mod.port_return == port_out.name)
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
                else
                  {
                    STRANGE ("port " << *name << " is not known at all");
                  }
              }
#undef STRANGE

            const path_t prefix (state.path_to_cpp());
            const path_t path (prefix / cpp_util::path::op() / mod.name);
            const std::string file_hpp (cpp_util::make::hpp (mod.function));
            const std::string file_cpp
              ( mod.code.isJust()
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
                                           );

              if (m[mod.name].find (fun_info) == m[mod.name].end())
                {
                  m[mod.name].insert (fun_info);
                }
              else
                {
                  state.warn ( warning::duplicate_external_function
                               ( mod.function
                               , mod.name
                               , state.file_in_progress()
                               )
                             );
                }
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

              for ( cincludes_type::const_iterator inc
                      (mod.cincludes.begin())
                  ; inc != mod.cincludes.end()
                  ; ++inc
                  )
                {
                  cpp_util::include (stream, *inc);
                }

              if (mod.code.isNothing())
                {
                  cpp_util::include (stream, "stdexcept");
                }

              namespace_open (stream, mod);

              mod_signature ( stream
                            , port_return
                            , ports_const, ports_mutable, ports_out, mod
                            );

              stream << std::endl << "      {" << std::endl;

              if (mod.code.isNothing())
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

      inline bool find_module_calls ( const state::type & state
                                    , function_type & f
                                    , fun_info_map & m
                                    )
      {
        f.contains_a_module_call
          = boost::apply_visitor
          ( visitor::find_module_calls<net_type>(state, f, m)
          , f.f
          );

        return f.contains_a_module_call;
      }

      // ***************************************************************** //

      inline void struct_to_cpp ( const struct_t & s
                                , const state::type & state
                                )
      {
        typedef boost::filesystem::path path_t;

        const path_t prefix (state.path_to_cpp());
        const path_t file
          (prefix / cpp_util::path::type() / cpp_util::make::hpp (s.name));

        util::check_no_change_fstream stream (state, file);

        state.verbose ("write to " + file.string());

        signature::cpp::cpp_header
          (stream, s.sig, s.name, s.path, cpp_util::path::type());

        stream.commit();
      }

      inline void structs_to_cpp ( const structs_type & structs
                                 , const state::type & state
                                 )
      {
        for ( structs_type::const_iterator pos (structs.begin())
            ; pos != structs.end()
            ; ++pos
            )
          {
            struct_to_cpp (*pos, state);
          }
      }

      // ***************************************************************** //

      inline void struct_to_cpp (const state::type &, const function_type &);

      namespace visitor
      {
        class transition_struct_to_cpp : public boost::static_visitor<void>
        {
        private:
          const state::type & state;

        public:
          transition_struct_to_cpp (const state::type & _state)
            : state (_state)
          {}

          void operator () (const function_type & f) const
          {
            struct_to_cpp (state, f);
          }

          template<typename T> void operator () (const T &) const {}
        };

        template<typename NET>
        class struct_to_cpp : public boost::static_visitor<void>
        {
        private:
          const state::type & state;

        public:
          struct_to_cpp (const state::type & _state)
            : state (_state)
          {}

          void operator () (const NET & n) const
          {
            if (!n.contains_a_module_call)
              {
                return;
              }

            structs_to_cpp (n.structs, state);

            for ( functions_type::const_iterator pos (n.functions().begin())
                ; pos != n.functions().end()
                ; ++pos
                )
              {
                xml::parse::type::struct_to_cpp (state, *pos);
              }

            for ( typename NET::transitions_type::const_iterator pos
                   (n.transitions().begin())
                ; pos != n.transitions().end()
                ; ++pos
                )
              {
                boost::apply_visitor ( transition_struct_to_cpp (state)
                                     , pos->f
                                     );
              }
          }

          template<typename T> void operator () (const T &) const {}
        };
      }

      // ***************************************************************** //

      inline void struct_to_cpp ( const state::type & state
                                , const function_type & f
                                )
      {
        if (!f.contains_a_module_call)
          {
            return;
          }

        structs_to_cpp (f.structs, state);

        boost::apply_visitor (visitor::struct_to_cpp<net_type>(state), f.f);
      }

      // ******************************************************************* //

      namespace dump
      {
        void dump ( xml_util::xmlstream &
                  , const function_type &
                  , const bool
                  );

        void dump ( xml_util::xmlstream &
                  , const transition_type &
                  );

        template<typename IT>
        inline void dumps ( xml_util::xmlstream & s
                          , IT pos
                          , const IT & end
                          )
        {
          for (; pos != end; ++pos)
            {
              ::xml::parse::type::dump::dump (s, *pos);
            }
        }

        template<typename IT, typename T>
        inline void dumps ( xml_util::xmlstream & s
                          , IT pos
                          , const IT & end
                          , const T & x
                          )
        {
          for (; pos != end; ++pos)
            {
              ::xml::parse::type::dump::dump (s, *pos, x);
            }
        }

        namespace visitor
        {
          template<typename NET>
          class function_dump : public boost::static_visitor<void>
          {
          private:
            xml_util::xmlstream & s;

          public:
            function_dump (xml_util::xmlstream & _s) : s (_s) {}

            template<typename T>
            void operator () (const T & x) const
            {
              ::xml::parse::type::dump::dump (s, x);
            }

            void operator () (const NET & net) const
            {
              s.open ("net");

              ::we::type::property::dump::dump (s, net.prop);

              dumps (s, net.structs.begin(), net.structs.end());
              dumps (s, net.templates().begin(), net.templates().end(), true);
              dumps (s, net.specializes().begin(), net.specializes().end());
              dumps (s, net.functions().begin(), net.functions().end(), false);
              dumps (s, net.places().begin(), net.places().end());
              dumps (s, net.transitions().begin(), net.transitions().end());

              s.close ();
            }
          };
        } // namespace visitor

        inline void dump_before_property ( xml_util::xmlstream & s
                                         , const function_type & f
                                         , const bool is_template = false
                                         )
        {
          s.open (is_template ? "template" : "defun");
          s.attr ("name", f.name);
          s.attr ("internal", f.internal);
        }

        inline void dump_after_property ( xml_util::xmlstream & s
                                        , const function_type & f
                                        )
        {
          dumps (s, f.structs.begin(), f.structs.end());

          xml::parse::type::dump::dump (s, f.requirements);

          dumps (s, f.in().begin(), f.in().end(), "in");
          dumps (s, f.out().begin(), f.out().end(), "out");
          dumps (s, f.tunnel().begin(), f.tunnel().end(), "tunnel");

          boost::apply_visitor (visitor::function_dump<net_type> (s), f.f);

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

        inline void dump ( xml_util::xmlstream & s
                         , const function_type & f
                         , const state::type & state
                         )
        {
          dump_before_property (s, f, false);

          state.dump_context (s);

          ::we::type::property::dump::dump (s, f.prop);

          dump_after_property (s, f);
        }

        inline void dump ( xml_util::xmlstream & s
                         , const function_type & f
                         , const bool is_template = false
                         )
        {
          dump_before_property (s, f, is_template);

          ::we::type::property::dump::dump (s, f.prop);

          dump_after_property (s, f);
        }
      } // namespace dump
    }
  }
}

#endif
