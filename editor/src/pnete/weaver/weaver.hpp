// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_TRAVERSE_WEAVER_HPP
#define _PNETE_TRAVERSE_WEAVER_HPP 1

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/link.hpp>
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

#include <we2/type/signature.hpp>

#include <boost/optional.hpp>
#include <boost/variant.hpp>

#if (  defined WNAME    \
    || defined WSIG     \
    || defined WSIGE    \
    || defined WEAVE    \
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

#define FUN(_name,_type,_var)                                           \
  template<typename State>                                              \
  static void _name (State*, const _type&);                             \
  template<typename State, typename Collection>                         \
  static void many_ ## _name (State* state, const Collection& collection) \
  {                                                                     \
    from::many (state, collection, from::_name<State>);                 \
  }                                                                     \
  template<typename State>                                              \
  static void _name (State * _state, const _type & _var)

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      namespace type
      {
        namespace transition
        {
          enum
            { first
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

        namespace visitor
        {
          template<typename State> class structure_structured;

          template<typename State>
          class structure_field : public boost::static_visitor<void>
          {
          public:
            explicit structure_field (State * state)
              : _state (state)
            {}

            void operator() (const std::pair<std::string, std::string>& f) const
            {
              WEAVE(structure::open) (f.first);
              WEAVE(structure::type) (f.second);
              WEAVE(structure::close) ();
            }
            void operator()
              (const pnet::type::signature::structured_type& s) const
            {
              boost::apply_visitor (structure_structured<State> (_state), s);
            }

          private:
            State * _state;
          };

          template<typename State>
          class structure_structured : public boost::static_visitor<void>
          {
          public:
            explicit structure_structured (State * state)
              : _state (state)
            {}

            void operator()
              (const std::pair< std::string
                              , pnet::type::signature::structure_type
                              >& s
              ) const
            {
              WEAVE(structure::open) (s.first);
              BOOST_FOREACH (const pnet::type::signature::field_type& f, s.second)
              {
                boost::apply_visitor (structure_field<State> (_state), f);
              }
              WEAVE(structure::close) ();
            }

          private:
            State * _state;
          };

          template<typename State>
          class structure : public boost::static_visitor<void>
          {
          public:
            explicit structure (State * state)
              : _state (state)
            {}

            void operator() (const ::std::string& t) const
            {
              WEAVE(structure::type) (t);
            }

            void operator()
              (const pnet::type::signature::structured_type& s) const
            {
              boost::apply_visitor (structure_structured<State> (_state), s);
            }

          private:
            State * _state;
          };

          template<typename State>
          class property : public boost::static_visitor<void>
          {
          private:
            State * _state;

          public:
            explicit property (State * state) : _state (state) {}

            void operator () (const ::we::type::property::value_type& v) const
            {
              WEAVE(property::value) (v);
            }

            void operator () (const ::we::type::property::type& props) const
            {
              for ( ::we::type::property::map_type::const_iterator
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

        FUN(property, ::we::type::property::map_type::const_iterator::value_type, prop)
        {
          WEAVE(property::open) (prop.first);
          boost::apply_visitor(visitor::property<State>(_state),prop.second);
          WEAVE(property::close)();
        }

        FUN(properties, ::we::type::property::type, props)
        {
          WEAVE(properties::open) (props);
          WEAVE(properties::close)();
        }

        FUN(structure, ::xml::parse::type::structure_type, s)
        {
          WEAVE(structure::open) (s.name());
          boost::apply_visitor( visitor::structure<State>(_state)
                              , s.signature()
                              );
          WEAVE(structure::close) ();
        }

        FUN(structs, ::xml::parse::type::structs_type, structs)
        {
          WEAVE(structs::open) (structs);
          WEAVE(structs::close)();
        }

        FUN(port, ::xml::parse::id::ref::port, id)
        {
          WEAVE(port::open) (id);
          const ::xml::parse::type::port_type& port (id.get());
          WEAVE(port::name) (port.name());
          WEAVE(port::type) (port.type());
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

        FUN(type_get, ::xml::parse::type::type_get_type::const_iterator::value_type, tg)
        {
          WEAVE(type_get::open) (tg);
          WEAVE(type_get::close)();
        }

        FUN(type_map, ::xml::parse::type::type_map_type::const_iterator::value_type, tm)
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
          WEAVE(mod::name) (mod.name());
          WEAVE(mod::fun) (::xml::parse::type::dump::dump_fun (mod));
          WEAVE(mod::cincludes) (mod.cincludes());
          WEAVE(mod::ldflags) (mod.ldflags());
          WEAVE(mod::cxxflags) (mod.cxxflags());
          WEAVE(mod::links) (mod.links());
          WEAVE(mod::code) (mod.code());
          WEAVE(mod::close)();
        }

        FUN(use, ::xml::parse::id::ref::use, id)
        {
          WEAVE(use::open) (id);
          WEAVE(use::name) (id.get().name());
          WEAVE(use::close)();
        }

        FUN(conditions, ::xml::parse::type::conditions_type, cs)
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
          WEAVE(function::fun) (fun.content());
          WEAVE(function::conditions) (fun.conditions());
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

        FUN(place, ::xml::parse::id::ref::place, place_id)
        {
          WEAVE(place::open) (place_id);
          const ::xml::parse::type::place_type& place (place_id.get());
          WEAVE(place::name) (place.name());
          WEAVE(place::type) (place.type());
          WEAVE(place::is_virtual) (place.is_virtual());
          WEAVE(place::properties) (place.properties());

          BOOST_FOREACH (const ::xml::parse::type::place_type::token_type& tok, place.tokens)
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
          WEAVE(transition::condition) (trans.conditions());
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
