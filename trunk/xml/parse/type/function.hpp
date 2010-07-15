// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_FUNCTION_HPP
#define _XML_PARSE_TYPE_FUNCTION_HPP

#include <parse/types.hpp>

#include <parse/util/maybe.hpp>
#include <parse/util/unique.hpp>
#include <parse/util/weparse.hpp>

#include <vector>

#include <boost/variant.hpp>
#include <boost/filesystem.hpp>

#include <we/type/literal.hpp>
#include <we/type/property.hpp>

#include <we/we.hpp>

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

      // ******************************************************************* //

      template<typename Map>
      typename Map::mapped_type
      get_pid (const Map & pid_of_place, const std::string name)
      {
        const typename Map::const_iterator pos (pid_of_place.find (name));

        if (pos == pid_of_place.end())
          {
            THROW_STRANGE ("missing place" << name << " in pid_of_place");
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
        const Fun & fun;

        typedef typename Activity::transition_type we_transition_type;

        typedef typename we_transition_type::place_type we_place_type;
        typedef typename we_transition_type::edge_type we_edge_type;

        typedef typename we_transition_type::expr_type we_expr_type;
        typedef typename we_transition_type::net_type we_net_type;
        typedef typename we_transition_type::mod_type we_mod_type;
        typedef typename we_transition_type::preparsed_cond_type we_cond_type;
        
        typedef typename we_transition_type::pid_t pid_t;

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
        function_synthesize ( const state::type & _state
                            , const Fun & _fun
                            )
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
            );

          add_ports (trans, fun.in(), we::type::PORT_IN);
          add_ports (trans, fun.out(), we::type::PORT_OUT);

          return trans;
        }

        we_transition_type operator () (const Net & net) const
        {
          we_net_type we_net;

          typedef boost::unordered_map<std::string, pid_t> pid_of_place_type;

          pid_of_place_type pid_of_place;

          for ( place_vec_type::const_iterator place (net.places().begin())
              ; place != net.places().end()
              ; ++place
              )
            {
              const signature::type type (net.type_of_place (*place));
              const pid_t pid
                ( we_net.add_place ( we_place_type ( place->name
                                                   , type
                                                   , place->prop
                                                   )
                                   )
                );

              pid_of_place[place->name] = pid;

              for ( value_vec_type::const_iterator val (place->values.begin())
                  ; val != place->values.end()
                  ; ++val
                  )
                {
                  token::put (we_net, pid, *val);
                }
            }

          we_edge_type e (0);

          for ( typename Net::transition_vec_type::const_iterator transition
                  (net.transitions().begin())
              ; transition != net.transitions().end()
              ; ++transition
              )
            {
              transition_synthesize< Activity
                                   , Net
                                   , transition_type
                                   , Fun
                                   , pid_of_place_type
                                   >
                (*transition, state, net, we_net, pid_of_place, e); 
            }

          // WORK HERE: add prop from net to fun

          we_transition_type trans
            ( name()
            , we_net
            , condition()
            , fun.internal.get_with_default (false)
            );

          add_ports (trans, fun.in(), we::type::PORT_IN, pid_of_place);
          add_ports (trans, fun.out(), we::type::PORT_OUT, pid_of_place);

          return trans;
        }
      };

      // ******************************************************************* //

      struct function_type
      {
      private:
        xml::util::unique<port_type> _in;
        xml::util::unique<port_type> _out;

      public:
        typedef boost::variant < expression_type
                               , mod_type
                               , boost::recursive_wrapper<net_type>
                               > type; 

        struct_vec_type structs;

        maybe<std::string> name;
        maybe<bool> internal;

        cond_vec_type cond;

        we::type::property::type prop;

        type f;

        boost::filesystem::path path;

        int level;

        xml::parse::struct_t::set_type structs_resolved;

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

        // ***************************************************************** //

        std::string condition (void) const
        {
          return cond.empty() 
            ? "true"
            : util::join (cond.begin(), cond.end(), " & ", "(", ")")
            ;
        }

        // ***************************************************************** //

        void push_in (const port_type & p)
        {
          port_type old;

          if (!_in.push (p, old))
            {
              throw error::duplicate_port ("in", p.name, path);
            }
        }

        void push_out (const port_type & p)
        {
          port_type old;

          if (!_out.push (p, old))
            {
              throw error::duplicate_port ("out", p.name, path);
            }
        }

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
                     , xml::parse::struct_t::forbidden_type & forbidden
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
        synthesize (const state::type & state) const
        {
          return boost::apply_visitor
            ( function_synthesize< Activity
                                 , net_type
                                 , function_type
                                 > (state, *this)
            , f
            );
        }
      };

      // ******************************************************************* //

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

        s << level(f.level+1) << "fun = ";

        boost::apply_visitor (visitor::show (s), f.f);

        s << std::endl;

        return s << level(f.level) << ") // function";
      }
    }
  }
}

#endif
