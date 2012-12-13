// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_TRAVERSE_WEAVER_HPP
#define _PNETE_TRAVERSE_WEAVER_HPP 1

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/use.hpp>

#include <we/type/signature.hpp>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#if (  defined WNAME    \
    || defined WSIG     \
    || defined WEAVE    \
    || defined XMLPARSE \
    || defined XMLTYPE  \
    || defined WETYPE   \
    || defined ITVAL    \
    || defined GENFUN   \
    || defined FUN      \
    )
#error "Macro already defined"
#endif

#define WNAME(_tag) ::fhg::pnete::weaver::type::_tag
#define WSIGE(_class,_tag) template<> void _class::weave< WNAME(_tag) > ()
#define WSIG(_class,_tag,_type,_var) \
        template<> \
        void _class::weave< WNAME(_tag), _type > (const _type & _var)
#define WEAVE(_tag) _state->template weave < WNAME(_tag) >

#define XMLPARSE(_x) ::xml::parse::_x
#define XMLTYPE(_type) XMLPARSE(type::_type)
#define WETYPE(_type) ::we::type::_type

#define ITVAL(_type) _type::const_iterator::value_type

#define GENFUN(_name,_type,_state,_var) \
        template<typename State> \
        static void _name (State * _state, const _type & _var)
#define FUN(_name,_type,_var) GENFUN(_name,_type,_state,_var)

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
          const ::xml::parse::id::ref::function& _fun;
          const XMLPARSE(state::key_values_t) & _context;

        public:
          function_context_type ( const ::xml::parse::id::ref::function& fun
                                , const XMLPARSE(state::key_values_t) & context
                                )
            : _fun (fun)
            , _context (context)
          {}

          const ::xml::parse::id::ref::function& fun () const { return _fun; }
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
            , structs, function, place_map, connection, condition
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

        namespace structure
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
            { first = structure::last + 1
            , open, close, name, type, place, properties
            , last
            };
        }

        namespace structs
        {
          enum
            { first = port::last + 1
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

        namespace tmpl
        {
          enum
            { first = conditions::last + 1
            , open, name, template_parameter, function, close
            , last
            };
        }

        namespace function
        {
          enum
            { first = tmpl::last + 1
            , open, close, name, internal, require, properties, structs
            , ports, fun, conditions
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

        namespace expression
        {
          enum
            { first = connection::last + 1
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
              WEAVE(token::literal::open) (t);
              WEAVE(token::literal::name) (t);
              WEAVE(token::literal::close)();
            }

            void operator () (const ::signature::structured_t & m) const
            {
              WEAVE(token::structured::open) (m);

              for ( ::signature::structured_t::const_iterator field (m.begin())
                  ; field != m.end()
                  ; ++field
                  )
                {
                  WEAVE(token::structured::field) (*field);
                }

              WEAVE(token::structured::close)();
            }
          };

          template<typename State>
          class structure : public boost::static_visitor<void>
          {
          private:
            State * _state;

          public:
            explicit structure (State * state) : _state (state) {}

            void operator () (const ::literal::type_name_t & t) const
            {
              WEAVE(structure::type) (t);
            }

            void operator () (const ::signature::structured_t & m) const
            {
              for ( ::signature::structured_t::const_iterator
                      field (m.begin()), end (m.end())
                  ; field != end
                  ; ++field
                  )
                {
                  WEAVE(structure::open) (field->first);
                  boost::apply_visitor (*this, field->second);
                  WEAVE(structure::close) ();
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
              WEAVE(property::value) (v);
            }

            void operator () (const WETYPE(property::type) & props) const
            {
              for ( WETYPE(property::map_type::const_iterator)
                      p (props.get_map().begin()), end (props.get_map().end())
                  ; p != end
                  ; ++p
                  )
                {
                  WEAVE(property::open) (p->first);
                  boost::apply_visitor(*this, p->second);
                  WEAVE(property::close) ();
                }
            }
          };
        } // namespace visitor

        FUN(property, ITVAL(WETYPE(property::map_type)), prop)
        {
          WEAVE(property::open) (prop.first);
          boost::apply_visitor(visitor::property<State>(_state),prop.second);
          WEAVE(property::close)();
        }

        FUN(properties, WETYPE(property::type), props)
        {
          WEAVE(properties::open) (props);
          WEAVE(properties::close)();
        }

        FUN(structure, XMLTYPE(structure_type), s)
        {
          WEAVE(structure::open) (s.name());
          boost::apply_visitor(visitor::structure<State>(_state),s.signature());
          WEAVE(structure::close) ();
        }

        FUN(structs, XMLTYPE(structs_type), structs)
        {
          WEAVE(structs::open) (structs);
          WEAVE(structs::close)();
        }

        FUN(port, ::xml::parse::id::ref::port, id)
        {
          WEAVE(port::open) (id);
          const ::xml::parse::type::port_type& port (id.get());
          WEAVE(port::name) (port.name());
          WEAVE(port::type) (port.type);
          WEAVE(port::place) (port.place);
          WEAVE(port::properties) (port.properties());
          WEAVE(port::close)();
        }

        FUN(expression_sequence, std::string, lines)
        {
          WEAVE(expression_sequence::open) (lines);

          const char b (';');
          std::string::const_iterator pos (lines.begin());
          const std::string::const_iterator end (lines.end());

          std::string line;

          while (pos != end)
            {
              switch (*pos)
                {
                case b:
                  WEAVE(expression_sequence::line) (line);
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
              WEAVE(expression_sequence::line)(line);
            }

          WEAVE(expression_sequence::close)();
        }

        FUN(type_get, ITVAL(XMLTYPE(type_get_type)), tg)
        {
          WEAVE(type_get::open) (tg);
          WEAVE(type_get::close)();
        }

        FUN(type_map, ITVAL(XMLTYPE(type_map_type)), tm)
        {
          WEAVE(type_map::open)(tm);
          WEAVE(type_map::close)();
        }

        FUN(specialize, ::xml::parse::id::ref::specialize, id)
        {
          WEAVE(specialize::open) (id);
          const ::xml::parse::type::specialize_type& spec (id.get());
          WEAVE(specialize::name) (spec.name());
          WEAVE(specialize::use) (spec.use);
          WEAVE(specialize::type_map) (spec.type_map);
          WEAVE(specialize::type_get) (spec.type_get);
          WEAVE(specialize::close)();
        }

        FUN(expression, ::xml::parse::id::ref::expression, id)
        {
          WEAVE(expression::open)(id);
          WEAVE(expression::close)();
        }

        FUN(module, ::xml::parse::id::ref::module, id)
        {
          WEAVE(mod::open) (id);
          const ::xml::parse::type::module_type& mod (id.get());
          WEAVE(mod::name) (mod.name);
          WEAVE(mod::fun) (XMLTYPE(dump::dump_fun(mod)));
          WEAVE(mod::cincludes) (mod.cincludes);
          WEAVE(mod::ldflags) (mod.ldflags);
          WEAVE(mod::cxxflags) (mod.cxxflags);
          WEAVE(mod::links) (mod.links);
          WEAVE(mod::code) (mod.code);
          WEAVE(mod::close)();
        }

        FUN(use, ::xml::parse::id::ref::use, id)
        {
          WEAVE(use::open) (id);
          WEAVE(use::name) (id.get().name());
          WEAVE(use::close)();
        }

        FUN(conditions, XMLTYPE(conditions_type), cs)
        {
          WEAVE(conditions::open) (cs);
          WEAVE(conditions::close)();
        }

        FUN(function_head, ::xml::parse::id::ref::function, id)
        {
          WEAVE(function::open) (id);
          const ::xml::parse::type::function_type& fun (id.get());
          WEAVE(function::name) (fun.name());
          WEAVE(function::internal) (fun.internal);
        }

        FUN(function_tail, ::xml::parse::id::ref::function, id)
        {
          const ::xml::parse::type::function_type& fun (id.get());
          WEAVE(function::properties) (fun.properties());
          WEAVE(function::structs) (fun.structs);
          WEAVE(function::require) (fun.requirements);
          WEAVE(function::ports) (fun.ports());
          WEAVE(function::fun) (fun.f);
          WEAVE(function::conditions) (fun.cond);
          WEAVE(function::close)();
        }

        FUN(function, ::xml::parse::id::ref::function, id)
        {
          from::function_head (_state, id);
          from::function_tail (_state, id);
        }

        FUN(tmpl, ::xml::parse::id::ref::tmpl, id)
        {
          WEAVE(tmpl::open) (id);
          const ::xml::parse::type::tmpl_type& t (id.get());
          WEAVE(tmpl::name) (t.name());
          WEAVE(tmpl::template_parameter) (t.tmpl_parameter());
          WEAVE(tmpl::function) (t.function());
          WEAVE(tmpl::close)();
        }

        FUN(context_key_value, ITVAL(XMLPARSE(state::key_values_t)), kv)
        {
          WEAVE(context::key_value) (kv);
        }

        FUN(context, XMLPARSE(state::key_values_t), context)
        {
          WEAVE(context::open) (context);
          WEAVE(context::close)();
        }

        FUN(function_context, WNAME(function_context_type), fs)
        {
          from::function_head (_state, fs.fun());
          from::context (_state, fs.context());
          from::function_tail (_state, fs.fun());
        }

        FUN(place, ::xml::parse::id::ref::place, place_id)
        {
          WEAVE(place::open) (place_id);
          const ::xml::parse::type::place_type& place (place_id.get());
          WEAVE(place::name) (place.name());
          WEAVE(place::type) (place.type);
          WEAVE(place::is_virtual) (place.is_virtual());
          WEAVE(place::properties) (place.properties());

          BOOST_FOREACH (const XMLTYPE(place_type::token_type)& tok, place.tokens)
          {
            WEAVE(place::token) (tok);
          }

          WEAVE(place::close)();
        }

        FUN(place_map, ::xml::parse::id::ref::place_map, id)
        {
          WEAVE(place_map::open) (id);
          const ::xml::parse::type::place_map_type& pm (id.get());
          WEAVE(place_map::place_virtual) (pm.place_virtual());
          WEAVE(place_map::place_real) (pm.place_real());
          WEAVE(place_map::properties) (pm.properties());
          WEAVE(place_map::close)();
        }

        FUN(connection, ::xml::parse::id::ref::connect, id)
        {
          WEAVE(connection::open) (id);
          const ::xml::parse::type::connect_type& connection (id.get());
          WEAVE(connection::port) (connection.port());
          WEAVE(connection::place) (connection.place());
          WEAVE(connection::close)();
        }

        FUN(transition, ::xml::parse::id::ref::transition, id)
        {
          WEAVE(transition::open) (id);
          const ::xml::parse::type::transition_type& trans (id.get());
          WEAVE(transition::name) (trans.name());
          WEAVE(transition::priority) (trans.priority);
          WEAVE(transition::internal) (trans.internal);
          WEAVE(transition::properties) (trans.properties());
          WEAVE(transition::structs) (trans.structs);
          WEAVE(transition::function) (trans.function_or_use());
          WEAVE(transition::place_map) (trans.place_map());
          WEAVE(transition::connection) (trans.connections());
          WEAVE(transition::condition) (trans.cond);
          WEAVE(transition::close)();
        }

        FUN(net, ::xml::parse::id::ref::net, net_id)
        {
          WEAVE(net::open) (net_id);
          const ::xml::parse::type::net_type& net (net_id.get());
          WEAVE(net::properties) (net.properties());
          WEAVE(net::structs) (net.structs);
          WEAVE(net::templates) (net.templates());
          WEAVE(net::specializes) (net.specializes());
          WEAVE(net::functions) (net.functions());
          WEAVE(net::places) (net.places());
          WEAVE(net::transitions) (net.transitions());
          WEAVE(net::close)();
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

        namespace
        {
          template<typename State>
          class visitor_from_variant : public boost::static_visitor<void>
          {
          private:
            State * _state;

          public:
            explicit visitor_from_variant (State * state)
              : _state (state)
            { }

#define DEREF_OP(_type)                                                    \
            void operator() (const ::xml::parse::id::ref::_type& id) const \
            {                                                              \
              from::_type (_state, id);                                    \
            }

            DEREF_OP (expression)
            DEREF_OP (function)
            DEREF_OP (module)
            DEREF_OP (net)
            DEREF_OP (use)

#undef DEREF_OP
          };
        }

        template<typename State, typename Variant>
          void variant (State* _state, const Variant& _variant)
        {
          boost::apply_visitor (visitor_from_variant<State> (_state), _variant);
        }
      } // namespace from
    } // namespace weaver
  }
}

#endif
