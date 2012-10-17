// mirko.rahn@itwm.fraunhofer.de

#ifndef _PNETE_TRAVERSE_WEAVER_HPP
#define _PNETE_TRAVERSE_WEAVER_HPP 1

#include <xml/parse/types.hpp>
#include <xml/parse/parser.hpp>

#include <we/type/signature.hpp>

#include <fhg/util/maybe.hpp>

#include <boost/variant.hpp>

#if (  defined WNAME    \
    || defined WSIG     \
    || defined WEAVE    \
    || defined WEAVEE   \
    || defined XMLPARSE \
    || defined XMLTYPE  \
    || defined WETYPE   \
    || defined MAYBE    \
    || defined ITVAL    \
    || defined GENFUN   \
    || defined FUN      \
    || defined SIG      \
    || defined FROM     \
    )
#error "Macro already defined"
#endif

#define WNAME(_tag) ::fhg::pnete::weaver::type::_tag
#define WSIGE(_class,_tag) template<> void _class::weave< WNAME(_tag) > ()
#define WSIG(_class,_tag,_type,_var) \
        template<> \
        void _class::weave< WNAME(_tag), _type > (const _type & _var)
#define WEAVE(_tag,_type) _state->template weave< WNAME(_tag), _type >
#define WEAVEE(_tag) _state->template weave < WNAME(_tag) >

#define XMLPARSE(_x) ::xml::parse::_x
#define XMLTYPE(_type) XMLPARSE(type::_type)
#define WETYPE(_type) ::we::type::_type
#define MAYBE(_type) ::fhg::util::maybe< _type >

#define ITVAL(_type) _type::const_iterator::value_type

#define GENFUN(_name,_type,_state,_var) \
        template<typename State> \
        static void _name (State * _state, const _type & _var)
#define FUN(_name,_type,_var) GENFUN(_name,_type,_state,_var)
#define SIG(_name,_type) GENFUN(_name,_type,,)

#define FROM(_fun) ::fhg::pnete::weaver::from::_fun

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      namespace type
      {
        struct function_context_type
        {
        private:
          const XMLTYPE(function_type) & _fun;
          const XMLPARSE(state::key_values_t) & _context;

        public:
          function_context_type ( const XMLTYPE(function_type) & fun
                                , const XMLPARSE(state::key_values_t) & context
                                )
            : _fun (fun)
            , _context (context)
          {}

          const XMLTYPE(function_type) & fun () const { return _fun; }
          const XMLPARSE(state::key_values_t) & context () const { return _context; }
        };

        namespace context
        {
          enum
            { first
            , open, close, key_value
            , last
            };
        }

        namespace transition
        {
          enum
            { first = context::first + 1
            , open, close, name, priority, internal, _inline, properties
            , structs, function, place_map, connect_read, connect_in
            , connect_out, condition
            , last
            };
        }

        namespace place
        {
          enum
            { first = transition::last + 1
            , open, close, name, type, is_virtual, token, properties
            , last
            };
        }

        namespace property
        {
          enum
            { first = place::last + 1
            , open, close, value
            , last
            };
        }

        namespace properties
        {
          enum
            { first = property::last + 1
            , open, close
            , last
            };
        }

        namespace _struct
        {
          enum
            { first = properties::last + 1
            , open, close, name, type
            , last
            };
        }

        namespace port
        {
          enum
            { first = _struct::last + 1
            , open, close, name, type, place, properties
            , last
            };
        }

        namespace cinclude
        {
          enum
            { first = port::last + 1
            , open, close
            , last
            };
        }

        namespace ldflag
        {
          enum
            { first = cinclude::last + 1
            , open, close
            , last
            };
        }

        namespace cxxflag
        {
          enum
            { first = ldflag::last + 1
            , open, close
            , last
            };
        }

        namespace link
        {
          enum
            { first = cxxflag::last + 1
            , open, close
            , last
            };
        }

        namespace structs
        {
          enum
            { first = link::last + 1
            , open, close
            , last
            };
        }

        namespace expression_sequence
        {
          enum
            { first = structs::last + 1
            , open, close, line
            , last
            };
        }

        namespace type_get
        {
          enum
            { first = expression_sequence::last + 1
            , open, close
            , last
            };
        }

        namespace type_map
        {
          enum
            { first = type_get::last + 1
            , open, close
            , last
            };
        }

        namespace specialize
        {
          enum
            { first = type_map::last + 1
            , open, close, name, use, type_map, type_get
            , last
            };
        }

        namespace conditions
        {
          enum
            { first = specialize::last + 1
            , open, close
            , last
            };
        }

        namespace function
        {
          enum
            { first = conditions::last + 1
            , open, close, name, internal, require, properties, structs
            , in, out, fun, conditions
            , last
            };
        }

        namespace place_map
        {
          enum
            { first = function::last + 1
            , open, close, place_virtual, place_real, properties
            , last
            };
        }

        namespace connection
        {
          enum
            { first = place_map::last + 1
            , open, close, place, port
            ,  last
            };
        }

        namespace requirement
        {
          enum
            { first = connection::last + 1
            , open, close, key, value
            , last
            };
        }

        namespace expression
        {
          enum
            { first = requirement::last + 1
            , open, close
            , last
            };
        }

        namespace mod
        {
          enum
            { first = expression::last + 1
            , open, close, name, fun, cincludes, ldflags, cxxflags, links, code
            , last
            };
        }

        namespace token
        {
          namespace literal
          {
            enum
              { first = mod::last + 1
              , open, close, name
              , last
              };
          }

          namespace structured
          {
            enum
              { first = literal::last + 1
              , open, close, field
              , last
              };
          }
        }

        namespace net
        {
          enum
            { first = token::structured::last + 1
            , open, close, properties, structs
            , templates, specializes, functions, places, transitions
            , last
            };
        }

        namespace use
        {
          enum
            { first = net::last + 1
            , open, close, name
            , last
            };
        }
      } // namespace type

      namespace from
      {
        SIG(place     , ITVAL(XMLTYPE(places_type)));
        SIG(function  , ITVAL(XMLTYPE(functions_type)));
        SIG(specialize, ITVAL(XMLTYPE(specializes_type)));

        SIG(conditions, XMLTYPE(conditions_type));
        SIG(structs   , XMLTYPE(structs_type));
        SIG(net       , XMLTYPE(net_type));

        SIG(cinclude  , ITVAL(XMLTYPE(cincludes_type)));
        SIG(connection, ITVAL(XMLTYPE(connections_type)));
        SIG(ldflag    , ITVAL(XMLTYPE(flags_type)));
        SIG(cxxflag   , ITVAL(XMLTYPE(flags_type)));
        SIG(link      , ITVAL(XMLTYPE(links_type)));
        SIG(port      , ITVAL(XMLTYPE(ports_type)));
        SIG(require   , ITVAL(XMLTYPE(requirements_type)));
        SIG(_struct   , ITVAL(XMLTYPE(structs_type)));
        SIG(transition, ITVAL(XMLTYPE(net_type::transitions_type)));
        SIG(type_get  , ITVAL(XMLTYPE(type_get_type)));
        SIG(type_map  , ITVAL(XMLTYPE(type_map_type)));
        SIG(place_map , ITVAL(XMLTYPE(place_maps_type)));

        SIG(properties, WETYPE(property::type));
        SIG(property, ITVAL(WETYPE(property::map_type)));

        SIG(structured, ITVAL(::signature::structured_t));

        SIG(expression_sequence, std::string);

        namespace visitor
        {
          template<typename State>
          class token : public boost::static_visitor<void>
          {
          private:
            State * _state;

          public:
            explicit token (State * state) : _state (state) {}

            void operator () (const ::literal::type_name_t & t) const
            {
              WEAVE(token::literal::open, ::literal::type_name_t)(t);
              WEAVE(token::literal::name, ::literal::type_name_t)(t);
              WEAVEE(token::literal::close)();
            }

            void operator () (const ::signature::structured_t & m) const
            {
              WEAVE(token::structured::open, ::signature::structured_t)(m);

              for ( ::signature::structured_t::const_iterator field (m.begin())
                  ; field != m.end()
                  ; ++field
                  )
                {
                  WEAVE( token::structured::field
                       , ITVAL(::signature::structured_t)
                       )(*field);
                }

              WEAVEE(token::structured::close)();
            }
          };

          template<typename State>
          class net_type : public boost::static_visitor<void>
          {
          private:
            State * _state;

          public:
            explicit net_type (State * state) : _state (state) {}

            void
            operator () (const XMLTYPE(expression_type) & exp) const
            {
              WEAVE(expression::open, XMLTYPE(expression_type))(exp);
              WEAVEE(expression::close)();
            }

            void operator () (const XMLTYPE(mod_type) & mod) const
            {
              WEAVE(mod::open, XMLTYPE(mod_type))(mod);
              WEAVE(mod::name, std::string)(mod.name);
              WEAVE(mod::fun, std::string)(XMLTYPE(dump::dump_fun(mod)));
              WEAVE(mod::cincludes, XMLTYPE(cincludes_type))(mod.cincludes);
              WEAVE(mod::ldflags, XMLTYPE(flags_type))(mod.ldflags);
              WEAVE(mod::cxxflags, XMLTYPE(flags_type))(mod.cxxflags);
              WEAVE(mod::links, XMLTYPE(links_type))(mod.links);
              WEAVE(mod::code, MAYBE(std::string))(mod.code);
              WEAVEE(mod::close)();
            }

            void operator () (const XMLTYPE(net_type) & net) const
            {
              FROM(net) (_state, net);
            }
          };

          template<typename State>
          class function_type : public boost::static_visitor<void>
          {
          private:
            State * _state;

          public:
            explicit function_type (State * state) : _state (state) {}

            void operator () (const XMLTYPE(use_type) & use) const
            {
              WEAVE(use::open, XMLTYPE(use_type))(use);
              WEAVE(use::name, std::string)(use.name);
              WEAVEE(use::close)();
            }

            void operator () (const XMLTYPE(function_type) & fun) const
            {
              FROM(function) (_state, fun);
            }
          };

          template<typename State>
          class _struct : public boost::static_visitor<void>
          {
          private:
            State * _state;

          public:
            explicit _struct (State * state) : _state (state) {}

            void operator () (const ::literal::type_name_t & t) const
            {
              WEAVE(_struct::type, ::literal::type_name_t)(t);
            }

            void operator () (const ::signature::structured_t & m) const
            {
              for ( ::signature::structured_t::const_iterator
                      field (m.begin()), end (m.end())
                  ; field != end
                  ; ++field
                  )
                {
                  WEAVE(_struct::open, std::string)(field->first);
                  boost::apply_visitor (*this, field->second);
                  WEAVEE(_struct::close) ();
                }
            }
          };

          template<typename State>
          class property : public boost::static_visitor<void>
          {
          private:
            State * _state;

          public:
            explicit property (State * state) : _state (state) {}

            void operator () (const WETYPE(property::value_type) & v) const
            {
              WEAVE(property::value, WETYPE(property::value_type))(v);
            }

            void operator () (const WETYPE(property::type) & props) const
            {
              for ( WETYPE(property::map_type::const_iterator)
                      p (props.get_map().begin()), end (props.get_map().end())
                  ; p != end
                  ; ++p
                  )
                {
                  WEAVE(property::open, WETYPE(property::key_type))(p->first);
                  boost::apply_visitor(*this, p->second);
                  WEAVEE(property::close) ();
                }
            }
          };
        } // namespace visitor

        FUN(property, ITVAL(WETYPE(property::map_type)), prop)
        {
          WEAVE(property::open, WETYPE(property::key_type))(prop.first);
          boost::apply_visitor(visitor::property<State>(_state),prop.second);
          WEAVEE(property::close) ();
        }

        FUN(properties, WETYPE(property::type), props)
        {
          WEAVE(properties::open, WETYPE(property::type))(props);
          WEAVEE(properties::close) ();
        }

        FUN(_struct, ITVAL(XMLTYPE(structs_type)), s)
        {
          WEAVE(_struct::open, std::string)(s.name);
          boost::apply_visitor(visitor::_struct<State>(_state),s.sig);
          WEAVEE(_struct::close) ();
        }

        FUN(structs, XMLTYPE(structs_type), structs)
        {
          WEAVE(structs::open, XMLTYPE(structs_type))(structs);
          WEAVEE(structs::close)();
        }

        FUN(port, ITVAL(XMLTYPE(ports_type)), port)
        {
          WEAVE(port::open, ITVAL(XMLTYPE(ports_type)))(port);
          WEAVE(port::name, std::string)(port.name);
          WEAVE(port::type, std::string)(port.type);
          WEAVE(port::place, MAYBE(std::string))(port.place);
          WEAVE(port::properties, WETYPE(property::type))(port.prop);
          WEAVEE(port::close)();
        }

        FUN(cinclude, ITVAL(XMLTYPE(cincludes_type)), cinclude)
        {
          WEAVE(cinclude::open, ITVAL(XMLTYPE(cincludes_type)))(cinclude);
          WEAVEE(cinclude::close)();
        }

        FUN(ldflag, ITVAL(XMLTYPE(flags_type)), flag)
        {
          WEAVE(ldflag::open, ITVAL(XMLTYPE(flags_type)))(flag);
          WEAVEE(ldflag::close)();
        }

        FUN(cxxflag, ITVAL(XMLTYPE(flags_type)), flag)
        {
          WEAVE(cxxflag::open, ITVAL(XMLTYPE(flags_type)))(flag);
          WEAVEE(cxxflag::close)();
        }

        FUN(link, ITVAL(XMLTYPE(links_type)), link)
        {
          WEAVE(link::open, ITVAL(XMLTYPE(links_type)))(link);
          WEAVEE(link::close)();
        }

        FUN(expression_sequence, std::string, lines)
        {
          WEAVE(expression_sequence::open, std::string)(lines);

          const char b (';');
          std::string::const_iterator pos (lines.begin());
          const std::string::const_iterator end (lines.end());

          std::string line;

          while (pos != end)
            {
              switch (*pos)
                {
                case b:
                  WEAVE(expression_sequence::line, std::string)(line);
                  line.clear();
                  ++pos;
                  while (pos != end && (isspace (*pos) || *pos == b))
                    {
                      ++pos;
                    }
                  break;
                default:
                  line += *pos;
                  ++pos;
                  break;
                }
            }

          if (line.size())
            {
              WEAVE(expression_sequence::line, std::string)(line);
            }

          WEAVEE(expression_sequence::close)();
        }

        FUN(type_get, ITVAL(XMLTYPE(type_get_type)), tg)
        {
          WEAVE(type_get::open, ITVAL(XMLTYPE(type_get_type)))(tg);
          WEAVEE(type_get::close)();
        }

        FUN(type_map, ITVAL(XMLTYPE(type_map_type)), tm)
        {
          WEAVE(type_map::open, ITVAL(XMLTYPE(type_map_type)))(tm);
          WEAVEE(type_map::close)();
        }

        FUN(specialize, ITVAL(XMLTYPE(specializes_type)), spec)
        {
          WEAVE(specialize::open, ITVAL(XMLTYPE(specializes_type)))(spec);
          WEAVE(specialize::name, std::string)(spec.name);
          WEAVE(specialize::use, std::string)(spec.use);
          WEAVE(specialize::type_map, XMLTYPE(type_map_type))(spec.type_map);
          WEAVE(specialize::type_get, XMLTYPE(type_get_type))(spec.type_get);
          WEAVEE(specialize::close)();
        }

        FUN(conditions, XMLTYPE(conditions_type), cs)
        {
          WEAVE(conditions::open, XMLTYPE(conditions_type))(cs);
          WEAVEE(conditions::close)();
        }

        FUN(function_head, ITVAL(XMLTYPE(functions_type)), fun)
        {
          WEAVE(function::open, ITVAL(XMLTYPE(functions_type)))(fun);
          WEAVE(function::name, MAYBE(std::string))(fun.name());
          WEAVE(function::internal, MAYBE(bool))(fun.internal);
        }

        FUN(function_tail, ITVAL(XMLTYPE(functions_type)), fun)
        {
          WEAVE(function::properties, WETYPE(property::type))(fun.prop);
          WEAVE(function::structs, XMLTYPE(structs_type))(fun.structs);
          WEAVE(function::require, XMLTYPE(requirements_type))
            (fun.requirements);
          WEAVE(function::in, XMLTYPE(ports_type))(fun.in());
          WEAVE(function::out, XMLTYPE(ports_type))(fun.out());
          WEAVE(function::fun, XMLTYPE(function_type::type))(fun.f);
          WEAVE(function::conditions, XMLTYPE(conditions_type))
            (fun.cond);
          WEAVEE(function::close)();
        }

        FUN(function, ITVAL(XMLTYPE(functions_type)), fun)
        {
          FROM(function_head) (_state, fun);
          FROM(function_tail) (_state, fun);
        }

        FUN(context_key_value, ITVAL(XMLPARSE(state::key_values_t)), kv)
        {
          WEAVE(context::key_value, ITVAL(XMLPARSE(state::key_values_t)))(kv);
        }

        FUN(context, XMLPARSE(state::key_values_t), context)
        {
          WEAVE(context::open, XMLPARSE(state::key_values_t))(context);
          WEAVEE(context::close)();
        }

        FUN(function_context, WNAME(function_context_type), fs)
        {
          FROM(function_head) (_state, fs.fun());
          FROM(context) (_state, fs.context());
          FROM(function_tail) (_state, fs.fun());
        }

        FUN(place, ITVAL(XMLTYPE(places_type)), place)
        {
          WEAVE(place::open, ITVAL(XMLTYPE(places_type)))(place);
          WEAVE(place::name, std::string)(place.name);
          WEAVE(place::type, std::string)(place.type);
          WEAVE(place::is_virtual, MAYBE(bool))(place.is_virtual());
          WEAVE(place::properties, WETYPE(property::type))(place.prop);

          for ( XMLTYPE(tokens_type::const_iterator) tok (place.tokens.begin())
              ; tok != place.tokens.end()
              ; ++tok
              )
            {
              WEAVE(place::token, ITVAL(XMLTYPE(tokens_type)))(*tok);
            }

          WEAVEE(place::close)();
        }

        FUN(place_map, ITVAL(XMLTYPE(place_maps_type)), pm)
        {
          WEAVE(place_map::open, ITVAL(XMLTYPE(place_maps_type)))(pm);
          WEAVE(place_map::place_virtual, std::string)(pm.place_virtual);
          WEAVE(place_map::place_real, std::string)(pm.place_real);
          WEAVE(place_map::properties, WETYPE(property::type))(pm.prop);
          WEAVEE(place_map::close)();
        }

        FUN(connection, ITVAL(XMLTYPE(connections_type)), connection)
        {
          WEAVE(connection::open, ITVAL(XMLTYPE(connections_type)))(connection);
          WEAVE(connection::port, std::string)(connection.port);
          WEAVE(connection::place, std::string)(connection.place);
          WEAVEE(connection::close)();
        }

        FUN(require, ITVAL(XMLTYPE(requirements_type)), req)
        {
          WEAVE(requirement::open, ITVAL(XMLTYPE(requirements_type)))(req);
          WEAVE(requirement::key, std::string)(req.first);
          WEAVE(requirement::value, bool)(req.second);
          WEAVEE(requirement::close)();
        }

        FUN(transition, ITVAL(XMLTYPE(net_type::transitions_type)), trans)
        {
          WEAVE(transition::open,ITVAL(XMLTYPE(net_type::transitions_type)))(trans);
          WEAVE(transition::name,std::string)(trans.name);
          WEAVE(transition::priority,MAYBE(petri_net::prio_t))
            (trans.priority);
          WEAVE(transition::internal, MAYBE(bool))(trans.internal);
          WEAVE(transition::properties, WETYPE(property::type))(trans.prop);
          WEAVE(transition::structs, XMLTYPE(structs_type))(trans.structs);
          WEAVE(transition::function, XMLTYPE(transition_type::function_or_use_type))
            (trans.function_or_use());
          WEAVE(transition::place_map, XMLTYPE(place_maps_type))
            (trans.place_map());
          WEAVE(transition::connect_read, XMLTYPE(connections_type))
            (trans.read());
          WEAVE(transition::connect_in, XMLTYPE(connections_type))(trans.in());
          WEAVE(transition::connect_out, XMLTYPE(connections_type))
            (trans.out());
          WEAVE(transition::condition, XMLTYPE(conditions_type))(trans.cond);
          WEAVEE(transition::close)();
        }

        FUN(net, XMLTYPE(net_type), net)
        {
          WEAVE(net::open, XMLTYPE(net_type))(net);
          WEAVE(net::properties, WETYPE(property::type))(net.prop);
          WEAVE(net::structs, XMLTYPE(structs_type))(net.structs);
          WEAVE(net::templates, XMLTYPE(templates_type))
            (net.templates());
          WEAVE(net::specializes, XMLTYPE(specializes_type))
            (net.specializes());
          WEAVE(net::functions, XMLTYPE(functions_type))
            (net.functions());
          WEAVE(net::places, XMLTYPE(net_type::places_type))(net.places());
          WEAVE(net::transitions, XMLTYPE(net_type::transitions_type))
            (net.transitions());
          WEAVEE(net::close)();
        }

        template<typename WEAVER, typename IT>
        void many ( WEAVER * weaver
                  , IT pos
                  , const IT & end
                  , void (*fun)
                    ( WEAVER *
                    , const typename std::iterator_traits<IT>::value_type &
                    )
                  )
        {
          while (pos != end)
            {
              fun (weaver, *pos);

              ++pos;
            }
        }

        template<typename WEAVER, typename COLLECTION>
        void many ( WEAVER * weaver
                  ,  const COLLECTION & collection
                  , void (*fun)
                    ( WEAVER *
                    , const typename std::iterator_traits<typename COLLECTION::const_iterator>::value_type &
                    )
                  )
        {
          many (weaver, collection.begin(), collection.end(), fun);
        }
      } // namespace from
    } // namespace weaver
  }
}

#endif
