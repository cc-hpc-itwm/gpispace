// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_FUNCTION_HPP
#define _XML_PARSE_TYPE_FUNCTION_HPP

#include <xml/parse/types.hpp>

#include <xml/parse/util/unique.hpp>
#include <xml/parse/util/weparse.hpp>
#include <xml/parse/util/property.hpp>

#include <vector>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

#include <we/type/literal/valid_name.hpp>
#include <we/type/property.hpp>

#include <we/we.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/maybe.hpp>

#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::vector<port_type> port_vec_type;
      typedef std::vector<std::string> cond_vec_type;

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

        typedef typename we_transition_type::pid_t pid_t;

        typedef boost::unordered_map<std::string, pid_t> pid_of_place_type;

        void add_ports ( we_transition_type & trans
                       , const port_vec_type & ports
                       , const we::type::PortDirection & direction
                       ) const
        {
          for ( port_vec_type::const_iterator port (ports.begin())
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
                       , const port_vec_type & ports
                       , const we::type::PortDirection & direction
                       , const Map & pid_of_place
                       ) const
        {
          for ( port_vec_type::const_iterator port (ports.begin())
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
            , fun.internal.get_with_default (false)
            , fun.prop
            );

          add_ports (trans, fun.in(), we::type::PORT_IN, pid_of_place);
          add_ports (trans, fun.out(), we::type::PORT_OUT, pid_of_place);

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

        struct_vec_type structs;

        fhg::util::maybe<std::string> name;
        fhg::util::maybe<bool> internal;

        cond_vec_type cond;

        we::type::property::type prop;

        type f;

        boost::filesystem::path path;

        int level;

        xml::parse::struct_t::set_type structs_resolved;

        bool was_template;

        // ***************************************************************** //

        const port_vec_type & in (void) const { return _in.elements(); }
        const port_vec_type & out (void) const { return _out.elements(); }

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

        // ***************************************************************** //

        xml::parse::struct_t::forbidden_type
        forbidden_below (void) const
        {
          xml::parse::struct_t::forbidden_type forbidden;

          for ( port_vec_type::const_iterator pos (in().begin())
              ; pos != in().end()
              ; ++pos
              )
            {
              forbidden[pos->type] = pos->name;
            }

          for ( port_vec_type::const_iterator pos (out().begin())
              ; pos != out().end()
              ; ++pos
              )
            {
              forbidden[pos->type] = pos->name;
            }

          return forbidden;
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
          for ( port_vec_type::const_iterator port (in().begin())
              ; port != in().end()
              ; ++port
              )
            {
              boost::apply_visitor
                (port_type_check<net_type> ("in", *port, path, state), f);
            }

          for ( port_vec_type::const_iterator port (out().begin())
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

          for ( std::vector<port_type>::iterator port (_in.elements().begin())
              ; port != _in.elements().end()
              ; ++port
              )
            {
              port->specialize (map,  state);
            }

          for ( std::vector<port_type>::iterator port (_out.elements().begin())
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

      // ******************************************************************* //

      inline
      std::ostream & operator << (std::ostream & s, const function_type & f)
      {
        s << level(f.level) << "function (" << std::endl;

        s << level(f.level+1)
          << "name = " << f.name
          << ", internal = " << f.internal
          << std::endl;
        s << level (f.level+1) << "path = " << f.path << std::endl;
        ;

        s << level(f.level+1) << "properties = " << std::endl;

        f.prop.writeTo (s, f.level+2);

        s << level(f.level+1) << "port_in = " << std::endl;

        for ( port_vec_type::const_iterator pos (f.in().begin())
            ; pos != f.in().end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level(f.level+1) << "port_out = " << std::endl;

        for ( port_vec_type::const_iterator pos (f.out().begin())
            ; pos != f.out().end()
            ; ++pos
            )
          {
            s << *pos << std::endl;
          }

        s << level(f.level+1) << "structs = " << std::endl;

        for ( struct_vec_type::const_iterator pos (f.structs.begin())
            ; pos != f.structs.end()
            ; ++pos
            )
          {
            type::struct_t deep (*pos);

            deep.level = f.level + 2;

            s << deep << std::endl;
          }

        s << level (f.level+1) << "resolved structs = " << std::endl;

        namespace st = xml::parse::struct_t;

        for ( st::set_type::const_iterator pos (f.structs_resolved.begin())
            ; pos != f.structs_resolved.end()
            ; ++pos
            )
          {
            type::struct_t deep (pos->second);

            deep.level = f.level + 2;

            s << deep << std::endl;
          }

        s << level(f.level+1) << "condition = " << std::endl;

        for ( cond_vec_type::const_iterator pos (f.cond.begin())
            ; pos != f.cond.end()
            ; ++pos
            )
          {
            s << level(f.level+2) << *pos << std::endl;
          }

        s << level(f.level+1) << "fun = " << std::endl;

        boost::apply_visitor (visitor::show (s, f.level+1), f.f);

        s << std::endl;

        return s << level(f.level) << ") // function";
      }
    }
  }
}

#endif
