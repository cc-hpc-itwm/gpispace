#include "StructureView.hpp"

#include <QStandardItem>
#include <QStandardItemModel>
#include <QWidget>
#include <QHeaderView>
#include <QString>

#include <xml/parse/types.hpp>
#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <we/type/signature.hpp>

#include <fhg/util/maybe.hpp>

#include <boost/variant.hpp>
#include <boost/format.hpp>

namespace fhg
{
  namespace pnete
  {
// #define INIT_WEAVER tv::weaver w (_state)
// #define STATE QStandardItem *
#define INIT_WEAVER tv::weaver w (_state)
#define STATE ::fhg::pnete::ui::tv::weaver

    namespace weaver
    {
      namespace type
      {
        typedef ::xml::parse::type::net_type::transitions_type::const_iterator::value_type transition_type;

        namespace transition
        {
          enum
            { first
            , open, close, name, priority, internal, properties, structs
            , function, place_map, connect_read, connect_in, connect_out
            , condition
            , last
            };
        }

        typedef ::xml::parse::type::places_type::const_iterator::value_type place_type;
        typedef ::xml::parse::type::tokens_type::const_iterator::value_type token_type;

        namespace place
        {
          enum
            { first = transition::last + 1
            , open, close, name, type, is_virtual, capacity, token, properties
            , last
            };
        }

        typedef ::we::type::property::map_type::const_iterator::value_type property_type;

        namespace property
        {
          enum
            { first = place::last + 1
            , open, close
            , last
            };
        }

        typedef ::we::type::property::type properties_type;

        namespace properties
        {
          enum
            { first = property::last + 1
            , open, close
            , last
            };
        }

        typedef ::signature::structured_t::const_iterator::value_type sig_structured_type;

        namespace sig_structured
        {
          enum
            { first = properties::last + 1
            , open, close
            , last
            };
        }

        typedef ::xml::parse::type::structs_type::const_iterator::value_type struct_type;

        namespace _struct
        {
          enum
            { first = sig_structured::last + 1
            , open, close
            , last
            };
        }

        typedef ::xml::parse::type::ports_type::const_iterator::value_type port_type;

        namespace port
        {
          enum
            { first = _struct::last + 1
            , open, close, name, type, place, properties
            , last
            };
        }

        typedef ::xml::parse::type::cincludes_type::const_iterator::value_type cinclude_type;

        namespace cinclude
        {
          enum
            { first = port::last + 1
            , open, close
            , last
            };
        }

        typedef ::xml::parse::type::links_type::const_iterator::value_type link_type;

        namespace link
        {
          enum
            { first = cinclude::last + 1
            , open, close
            , last
            };
        }

        typedef ::xml::parse::type::structs_type structs_type;

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

        typedef ::xml::parse::type::type_get_type::const_iterator::value_type type_get_type;

        namespace type_get
        {
          enum
            { first = expression_sequence::last + 1
            , open, close
            , last
            };
        }

        typedef ::xml::parse::type::type_map_type::const_iterator::value_type type_map_type;

        namespace type_map
        {
          enum
            { first = type_get::last + 1
            , open, close
            , last
            };
        }

        typedef ::xml::parse::type::net_type::specializes_type::const_iterator::value_type specialize_type;

        namespace specialize
        {
          enum
            { first = type_map::last + 1
            , open, close, name, use, type_map, type_get
            , last
            };
        }

        typedef ::xml::parse::type::conditions_type conditions_type;

        namespace conditions
        {
          enum
            { first = specialize::last + 1
            , open, close
            , last
            };
        }

        typedef ::xml::parse::type::net_type::functions_type::const_iterator::value_type function_type;

        namespace function
        {
          enum
            { first = conditions::last + 1
            , open, close, name, internal, require, properties, structs
            , in, out, fun, conditions
            , last
            };
        }

        typedef ::xml::parse::type::place_maps_type::const_iterator::value_type place_map_type;

        namespace place_map
        {
          enum
            { first = function::last + 1
            , open, close, place_virtual, place_real, properties
            , last
            };
        }

        typedef ::xml::parse::type::connections_type::const_iterator::value_type connection_type;

        namespace connection
        {
          enum
            { first = place_map::last + 1
            , open, close, place, port
            ,  last
            };
        }

        typedef ::xml::parse::type::requirements_type::const_iterator::value_type requirement_type;

        namespace requirement
        {
          enum
            { first = connection::last + 1
            , open, close, key, value
            , last
            };
        }
      } // namespace type
    } // namespace weaver

    namespace ui
    {
      namespace tv
      {
        class weaver
        {
        public:
          explicit weaver (QStandardItem * state) : _state (state) {}

          template<int Type, typename T> void weave (const T & x) {}
          template<int Type> void weave () {}

          template<typename T> weaver append (const T & x) const
          {
            QStandardItem* x_item (new QStandardItem (x));
            x_item->setEditable (false);
            _state->appendRow (x_item);
            return weaver (x_item);
          }

          QStandardItem * state () const { return _state; }

          template<typename IT>
          void from_xs
          ( const QString & header
          , IT pos
          , const IT & end
          , void (*fun) ( const weaver &
                        , const typename std::iterator_traits<IT>::value_type &
                        )
          ) const
          {
            if (pos != end)
              {
                weaver w (append (header));

                while (pos != end)
                  {
                    fun (w, *pos);

                    ++pos;
                  }
              }
          }

          template<typename Coll>
          void from_xs
          ( const std::string & header
          , const Coll & coll
          , void (*fun) ( const weaver &
                        , const typename std::iterator_traits<typename Coll::const_iterator>::value_type &
                        )
          ) const
          {
            from_xs (QString (header.c_str()), coll.begin(), coll.end(), fun);
          }

        private:
          QStandardItem* _state;
        };

        template<>
        weaver weaver::append<std::string> (const std::string & str) const
        {
          return append (QString (str.c_str()));
        }

        template<>
        weaver weaver::append<boost::format> (const boost::format & f) const
        {
          return append (f.str());
        }

#define WNAME(_tag) fhg::pnete::weaver::type::_tag
#define WSIG(_tag,_type,_var) \
        template<> void weaver::weave< WNAME(_tag), _type > (const _type & _var)
#define WSIGN(_tag,_type,_var) WSIG(_tag,WNAME(_type),_var)
#define WWEAVE(_tag,_type) w.weave< WNAME(_tag), _type >
#define WWEAVEN(_tag,_type) WWEAVE(_tag,WNAME(_type))
#define WWEAVEE(_tag) w.weave< WNAME(_tag) >
      }

      namespace detail
      {
#if (  defined IT      \
    || defined FUNT    \
    || defined FUN     \
    || defined FUN_IT  \
    || defined SIG     \
    || defined SIG_IT  \
    )
#error "Macro already defined"
#endif

#define IT(_type) _type::const_iterator::value_type

#define FUN(_name,_type,_var) \
        template<typename State> \
        static void from_ ## _name (State _state, const _type & _var)
#define FUN_IT(_name,_type,_var) FUN(_name,IT(_type),_var)
#define SIG(_name,_type) FUN(_name,_type,)
#define SIG_IT(_name,_type) SIG(_name,IT(_type))

        SIG_IT(place     , ::xml::parse::type::places_type);
        SIG_IT(function  , ::xml::parse::type::net_type::functions_type);
        SIG_IT(specialize, ::xml::parse::type::net_type::specializes_type);

        SIG(conditions, ::xml::parse::type::conditions_type);
        SIG(structs   , ::xml::parse::type::structs_type);

        SIG_IT(cinclude  , ::xml::parse::type::cincludes_type);
        SIG_IT(connection, ::xml::parse::type::connections_type);
        SIG_IT(link      , ::xml::parse::type::links_type);
        SIG_IT(port      , ::xml::parse::type::ports_type);
        SIG_IT(require   , ::xml::parse::type::requirements_type);
        SIG_IT(struct    , ::xml::parse::type::structs_type);
        SIG_IT(transition, ::xml::parse::type::net_type::transitions_type);
        SIG_IT(type_get  , ::xml::parse::type::type_get_type);
        SIG_IT(type_map  , ::xml::parse::type::type_map_type);
        SIG_IT(place_map , ::xml::parse::type::place_maps_type);

        SIG(properties, ::we::type::property::type);
        SIG_IT(property, ::we::type::property::map_type);

        SIG_IT(structured, ::signature::structured_t);

        SIG(expression_sequence, std::string);

        namespace visitor
        {
          template<typename State>
          class from_token : public boost::static_visitor<void>
          {
          private:
            State _state;

          public:
            from_token (State state) : _state (state) {}

            void operator () (const ::literal::type_name_t & t) const
            {
              _state.append (boost::format("%s") % t);
            }

            void operator () (const ::signature::structured_t & map) const
            {
              for ( ::signature::structured_t::const_iterator field (map.begin())
                  ; field != map.end()
                  ; ++field
                  )
                {
                  boost::apply_visitor
                    ( from_token (_state.append (field->first))
                    , field->second
                    );
                }
            }
          };

          template<typename State>
          class from_net_type : public boost::static_visitor<void>
          {
          private:
            State _state;

          public:
            from_net_type (State state) : _state (state) {}

            void
            operator () (const ::xml::parse::type::expression_type & exp) const
            {
              _state.from_xs ( "expression"
                             , exp.expressions
                             , from_expression_sequence
                             );
            }

            void operator () (const ::xml::parse::type::mod_type & mod) const
            {
              State s1 ( _state
                       . append ("module")
                       . append ( boost::format ("%s -> %s")
                                % mod.name
                                % ::xml::parse::type::dump::dump_fun (mod)
                                )
                       );

              s1.from_xs ("cinclude", mod.cincludes, from_cinclude);
              s1.from_xs ("link", mod.links, from_link);

              if (mod.code)
                {
                  s1.append ("code").append(*mod.code);
                }
            }

            void operator () (const ::xml::parse::type::net_type & net) const
            {
              State s1 (_state.append ("net"));

              from_properties (s1, net.prop);
              from_structs (s1, net.structs);
              s1.from_xs ("template", net.templates(), from_function);
              s1.from_xs ("specialize", net.specializes(), from_specialize);
              s1.from_xs ("function", net.functions(), from_function);
              s1.from_xs ("place", net.places(), from_place);
              s1.from_xs ("transition", net.transitions(), from_transition);
            }
          };

          template<typename State>
          class from_function_type : public boost::static_visitor<void>
          {
          private:
            State _state;

          public:
            from_function_type (State state) : _state (state) {}

            void operator () (const ::xml::parse::type::use_type & use) const
            {
              _state.append (boost::format("use: %s") % use.name);
            }

            void operator () (const ::xml::parse::type::function_type & fun) const
            {
              from_function (_state, fun);
            }
          };

          template<typename State>
          class from_signature : public boost::static_visitor<void>
          {
          private:
            const std::string & _name;
            State _state;

          public:
            from_signature (const std::string & name, State state)
              : _name (name)
              , _state (state)
            {}

            void operator () (const ::literal::type_name_t & t) const
            {
              _state.append (boost::format("%s :: %s") % _name % t);
            }

            void operator () (const ::signature::structured_t & map) const
            {
              _state.from_xs (_name, map, from_structured);
            }
          };

          template<typename State>
          class from_property : public boost::static_visitor<void>
          {
          private:
            State _state;
            const ::we::type::property::key_type & _key;

          public:
            from_property ( State state
                          , const ::we::type::property::key_type & key
                          )
              : _state (state)
              , _key (key)
            {}

            void operator () (const ::we::type::property::value_type & v) const
            {
              _state.append (boost::format("%s: %s") % _key % v);
            }

            void operator () (const ::we::type::property::type & prop) const
            {
              from_properties (_state.append (_key), prop);
            }
          };
        } // namespace visitor
      } // namespace detail

      namespace tv {
        WSIG (transition::name, std::string, name)
        {
          _state = append (name).state();
        }
        WSIG(transition::priority, fhg::util::maybe<petri_net::prio_t>, prio)
        {
          if (prio)
            {
              append (boost::format ("priority: %i") % *prio);
            }
        }
        WSIG (transition::internal, fhg::util::maybe<bool>, internal)
        {
          if (internal)
            {
              append ( boost::format ("internal: %i")
                     % (*internal ? "true" : "false")
                     );
            }
        }
        WSIG(transition::properties, ::we::type::property::type, prop)
        {
          ui::detail::from_properties (_state, prop);
        }
        WSIG(transition::structs, ::xml::parse::type::structs_type, structs)
        {
          ui::detail::from_structs (_state, structs);
        }
        WSIG(transition::function, ::xml::parse::type::transition_type::f_type, fun)
        {
          boost::apply_visitor
            ( ui::detail::visitor::from_function_type<STATE> (*this)
            , fun
            );
        }
        WSIG(transition::place_map, ::xml::parse::type::place_maps_type, pm)
        {
          from_xs ("place-map", pm, ui::detail::from_place_map);
        }
        WSIG(transition::connect_read, ::xml::parse::type::connections_type, cs)
        {
          from_xs ("connect-read", cs, ui::detail::from_connection);
        }
        WSIG(transition::connect_in, ::xml::parse::type::connections_type, cs)
        {
          from_xs ("connect-in", cs, ui::detail::from_connection);
        }
        WSIG(transition::connect_out, ::xml::parse::type::connections_type, cs)
        {
          from_xs ("connect-out", cs, ui::detail::from_connection);
        }
        WSIG(transition::condition, ::xml::parse::type::conditions_type, cond)
        {
          ui::detail::from_conditions (*this, cond);
        }

        WSIGN(place::open, place_type, place)
        {
          _state = append ("<<place>>").state();
        }
        WSIG(place::name, std::string, name)
        {
          _state->setText (QString (name.c_str()));
        }
        WSIG(place::type, std::string, type)
        {
          _state->setText ( _state->text()
                          . append(" :: ")
                          . append(type.c_str())
                          );
        }
        WSIG(place::is_virtual, fhg::util::maybe<bool>, is_virtual)
        {
          if (is_virtual)
            {
              append ( boost::format ("virtual: %s")
                     % (*is_virtual ? "true" : "false")
                     );
            }
        }
        WSIG(place::capacity, fhg::util::maybe<petri_net::capacity_t>, capacity)
        {
          if (capacity)
            {
              append (boost::format ("capacity: %i") % *capacity);
            }
        }
        WSIGN(place::token, token_type, token)
        {
          boost::apply_visitor
            ( ui::detail::visitor::from_token<STATE> (append ("token"))
            , token
            );
        }
        WSIG(place::properties, ::we::type::property::type, prop)
        {
          ui::detail::from_properties (*this, prop);
        }

        WSIGN(property::open, property_type, prop)
        {
          boost::apply_visitor
            ( ui::detail::visitor::from_property<STATE> ( *this
                                                        , prop.first
                                                        )
            , prop.second
            );
        }

        WSIGN(properties::open, properties_type, props)
        {
          from_xs ( "property"
                  , props.get_map()
                  , ui::detail::from_property
                  );
        }

        WSIGN(sig_structured::open, sig_structured_type, sig)
        {
          boost::apply_visitor
            ( ui::detail::visitor::from_signature<STATE> ( sig.first
                                                         , *this
                                                         )
            , sig.second
            );
        }

        WSIGN(_struct::open, struct_type, s)
        {
          boost::apply_visitor
            ( ui::detail::visitor::from_signature<STATE> ( s.name
                                                         , *this
                                                         )
            , s.sig
            );
        }

        WSIGN(port::open, port_type, port)
        {
          _state = append ("<<port>>").state();
        }
        WSIG(port::name, std::string, name)
        {
          _state->setText (QString (name.c_str()));
        }
        WSIG(port::type, std::string, type)
        {
          _state->setText (_state->text().append(" :: ").append(type.c_str()));
        }
        WSIG(port::place, fhg::util::maybe<std::string>, place)
        {
          if (place)
            {
              append (boost::format("place: %s") % *place);
            }
        }
        WSIG(port::properties, ::we::type::property::type, prop)
        {
          ui::detail::from_properties (*this, prop);
        }

        WSIGN(cinclude::open, cinclude_type, cinclude)
        {
          append (cinclude);
        }

        WSIGN(link::open, link_type, link)
        {
          append (link);
        }

        WSIGN(structs::open, structs_type, structs)
        {
          from_xs ("struct", structs, ui::detail::from_struct);
        }

        WSIG(expression_sequence::line, std::string, line)
        {
          append (line);
        }

        WSIGN(type_get::open, type_get_type, tg)
        {
          append (tg);
        }

        WSIGN(type_map::open, type_map_type, tm)
        {
          append (boost::format("%s => %s") % tm.first % tm.second);
        }

        WSIGN(specialize::open, specialize_type, specialize)
        {
          _state = append ("<<specialize>>").state();
        }
        WSIG(specialize::name, std::string, name)
        {
          _state->setText (QString (name.c_str()));
        }
        WSIG(specialize::use, std::string, use)
        {
          _state->setText (_state->text().append(" use ").append(use.c_str()));
        }
        WSIG(specialize::type_map, ::xml::parse::type::type_map_type, tm)
        {
          from_xs ("type_map", tm, ui::detail::from_type_map);
        }
        WSIG(specialize::type_get, ::xml::parse::type::type_get_type, tg)
        {
          from_xs ("type_get", tg, ui::detail::from_type_get);
        }

        WSIGN(conditions::open, conditions_type, cs)
        {
          from_xs ( "condition"
                  , cs
                  , ui::detail::from_expression_sequence
                  );
        }

        WSIGN(function::open, function_type, fun)
        {
          _state = append ("<<function>>").state();
        }
        WSIG(function::name, fhg::util::maybe<std::string>, name)
        {
          if (name)
            {
              _state->setText (QString ((*name).c_str()));
            }
        }
        WSIG(function::internal, fhg::util::maybe<bool>, internal)
        {
          if (internal)
            {
              append ( boost::format ("internal: %s")
                     % (*internal ? "true" : "false")
                     );
            }
        }
        WSIG(function::require, ::xml::parse::type::requirements_type, reqs)
        {
          from_xs ("require", reqs, ui::detail::from_require);
        }
        WSIG(function::properties, ::we::type::property::type, prop)
        {
          ui::detail::from_properties (*this, prop);
        }
        WSIG(function::structs, ::xml::parse::type::structs_type, structs)
        {
          ui::detail::from_structs (*this, structs);
        }
        WSIG(function::in, ::xml::parse::type::ports_type, ports)
        {
          from_xs ("in", ports, ui::detail::from_port);
        }
        WSIG(function::out, ::xml::parse::type::ports_type, ports)
        {
          from_xs ("out", ports, ui::detail::from_port);
        }
        WSIG(function::fun, ::xml::parse::type::function_type::type, fun)
        {
          boost::apply_visitor
            ( ui::detail::visitor::from_net_type<STATE> (*this)
            , fun
            );
        }
        WSIG(function::conditions, ::xml::parse::type::conditions_type, cs)
        {
          ui::detail::from_conditions (*this, cs);
        }

        WSIGN(place_map::open, place_map_type, pm)
        {
          _state = append ("<<place_map>>").state();
        }
        WSIG(place_map::place_virtual, std::string, name)
        {
          _state->setText (QString ("virtual: ").append(name.c_str()));
        }
        WSIG(place_map::place_real, std::string, name)
        {
          _state->setText ( _state->text()
                          . append (" <-> real: ")
                          . append (name.c_str())
                          );
        }
        WSIG(place_map::properties, ::we::type::property::type, prop)
        {
          ui::detail::from_properties (*this, prop);
        }

        WSIGN(connection::open, connection_type, connection)
        {
          _state = append ("<<connection>>").state();
        }
        WSIG(connection::port, std::string, port)
        {
          _state->setText (QString ("port: ").append (port.c_str()));
        }
        WSIG(connection::place, std::string, place)
        {
          _state->setText ( _state->text()
                          . append (" -> ")
                          . append (place.c_str())
                          );
        }

        WSIGN(requirement::open, requirement_type, req)
        {
          _state = append ("requirement").state();
        }
        WSIG(requirement::key, std::string, key)
        {
          _state->setText (_state->text().append(key.c_str()));
        }
        WSIG(requirement::value, bool, val)
        {
          _state->setText ( _state->text()
                          . append (": ")
                          . append (val ? "true" : "false")
                          );
        }
      } // namespace tv

      namespace detail
      {
        FUN_IT(property, ::we::type::property::map_type, prop)
        {
          INIT_WEAVER;;

          WWEAVEN(property::open, property_type)(prop);
          WWEAVEE(property::close)();
        }

        FUN(properties, ::we::type::property::type, props)
        {
          INIT_WEAVER;;

          WWEAVEN(properties::open, properties_type)(props);
          WWEAVEE(properties::close)();
        }

        FUN_IT(structured, ::signature::structured_t, sig)
        {
          INIT_WEAVER;;

          WWEAVEN(sig_structured::open, sig_structured_type)(sig);
          WWEAVEE(sig_structured::close)();
        }

        FUN_IT(struct, ::xml::parse::type::structs_type, s)
        {
          INIT_WEAVER;;

          WWEAVEN(_struct::open, struct_type)(s);
          WWEAVEE(_struct::close)();
        }

        FUN_IT(port, ::xml::parse::type::ports_type, port)
        {
          INIT_WEAVER;;

          WWEAVEN(port::open, port_type)(port);
          WWEAVE(port::name, std::string)(port.name);
          WWEAVE(port::type, std::string)(port.type);
          WWEAVE(port::place, fhg::util::maybe<std::string>)(port.place);
          WWEAVE(port::properties, ::we::type::property::type)(port.prop);
          WWEAVEE(port::close)();
        }

        FUN_IT(cinclude, ::xml::parse::type::cincludes_type, cinclude)
        {
          INIT_WEAVER;;

          WWEAVEN(cinclude::open, cinclude_type)(cinclude);
          WWEAVEE(cinclude::close)();
        }

        FUN_IT(link, ::xml::parse::type::links_type, link)
        {
          INIT_WEAVER;;

          WWEAVEN(link::open, link_type)(link);
          WWEAVEE(link::close)();
        }

        FUN(structs, ::xml::parse::type::structs_type, structs)
        {
          INIT_WEAVER;;

          WWEAVEN(structs::open, structs_type)(structs);
          WWEAVEE(structs::close)();
        }

        FUN(expression_sequence, std::string, lines)
        {
          INIT_WEAVER;;

          WWEAVE(expression_sequence::open, std::string)(lines);

          const char b (';');
          std::string::const_iterator pos (lines.begin());
          const std::string::const_iterator end (lines.end());

          std::string line;

          while (pos != end)
            {
              switch (*pos)
                {
                case b:
                  WWEAVE(expression_sequence::line, std::string)(line);
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
              WWEAVE(expression_sequence::line, std::string)(line);
            }

          WWEAVEE(expression_sequence::close)();
        }

        FUN_IT(type_get, ::xml::parse::type::type_get_type, tg)
        {
          INIT_WEAVER;;

          WWEAVEN(type_get::open, type_get_type)(tg);
          WWEAVEE(type_get::close)();
        }

        FUN_IT(type_map, ::xml::parse::type::type_map_type, tm)
        {
          INIT_WEAVER;;

          WWEAVEN(type_map::open, type_map_type)(tm);
          WWEAVEE(type_map::close)();
        }

        FUN_IT(specialize, ::xml::parse::type::net_type::specializes_type, spec)
        {
          INIT_WEAVER;;

          WWEAVEN(specialize::open, specialize_type)(spec);
          WWEAVE(specialize::name, std::string)(spec.name);
          WWEAVE(specialize::use, std::string)(spec.use);
          WWEAVE(specialize::type_map, ::xml::parse::type::type_map_type)(spec.type_map);
          WWEAVE(specialize::type_get, ::xml::parse::type::type_get_type)(spec.type_get);
          WWEAVEE(specialize::close)();
        }


        FUN(conditions, ::xml::parse::type::conditions_type, cs)
        {
          INIT_WEAVER;;

          WWEAVEN(conditions::open, conditions_type)(cs);
          WWEAVEE(conditions::close)();
        }

        FUN_IT(function, ::xml::parse::type::net_type::functions_type, fun)
        {
          INIT_WEAVER;;

          WWEAVEN(function::open, function_type)(fun);
          WWEAVE(function::name, fhg::util::maybe<std::string>)(fun.name);
          WWEAVE(function::internal, fhg::util::maybe<bool>)(fun.internal);
          WWEAVE(function::require, ::xml::parse::type::requirements_type)
            (fun.requirements);
          WWEAVE(function::properties, ::we::type::property::type)(fun.prop);
          WWEAVE(function::structs, ::xml::parse::type::structs_type)
            (fun.structs);
          WWEAVE(function::in, ::xml::parse::type::ports_type)(fun.in());
          WWEAVE(function::out, ::xml::parse::type::ports_type)(fun.out());
          WWEAVE(function::fun, ::xml::parse::type::function_type::type)(fun.f);
          WWEAVE(function::conditions, ::xml::parse::type::conditions_type)
            (fun.cond);
          WWEAVEE(function::close)();
        }

        FUN_IT(place, ::xml::parse::type::places_type, place)
        {
          INIT_WEAVER;;

          WWEAVEN(place::open, place_type)(place);
          WWEAVE(place::name, std::string)(place.name);
          WWEAVE(place::type, std::string)(place.type);
          WWEAVE(place::is_virtual, fhg::util::maybe<bool>)(place.is_virtual);
          WWEAVE(place::capacity, fhg::util::maybe<petri_net::capacity_t>)
            (place.capacity);
          WWEAVE(place::properties, ::we::type::property::type)(place.prop);

          for ( ::xml::parse::type::tokens_type::const_iterator
                  tok (place.tokens.begin())
              ; tok != place.tokens.end()
              ; ++tok
              )
            {
              WWEAVEN(place::token, token_type)(*tok);
            }

          WWEAVEE(place::close)();
        }

        FUN_IT(place_map, ::xml::parse::type::place_maps_type, pm)
        {
          INIT_WEAVER;;

          WWEAVEN(place_map::open, place_map_type)(pm);
          WWEAVE(place_map::place_virtual, std::string)(pm.place_virtual);
          WWEAVE(place_map::place_real, std::string)(pm.place_real);
          WWEAVE(place_map::properties, ::we::type::property::type)(pm.prop);
          WWEAVEE(place_map::close)();
        }

        FUN_IT(connection, ::xml::parse::type::connections_type, connection)
        {
          INIT_WEAVER;;

          WWEAVEN(connection::open, connection_type)(connection);
          WWEAVE(connection::port, std::string)(connection.port);
          WWEAVE(connection::place, std::string)(connection.place);
          WWEAVEE(connection::close)();
        }

        FUN_IT(require, ::xml::parse::type::requirements_type, req)
        {
          INIT_WEAVER;;

          WWEAVEN(requirement::open, requirement_type)(req);
          WWEAVE(requirement::key, std::string)(req.first);
          WWEAVE(requirement::value, bool)(req.second);
          WWEAVEE(requirement::close)();
        }

        FUN_IT(transition, ::xml::parse::type::net_type::transitions_type, trans)
        {
          INIT_WEAVER;;

          WWEAVEN(transition::open,transition_type)(trans);
          WWEAVE(transition::name,std::string)(trans.name);
          WWEAVE(transition::priority,fhg::util::maybe<petri_net::prio_t>)
            (trans.priority);
          WWEAVE(transition::internal, fhg::util::maybe<bool>)(trans.internal);
          WWEAVE(transition::properties, ::we::type::property::type)
            (trans.prop);
          WWEAVE(transition::structs, ::xml::parse::type::structs_type)
            (trans.structs);
          WWEAVE( transition::function
                , ::xml::parse::type::transition_type::f_type
                )(trans.f);
          WWEAVE(transition::place_map, ::xml::parse::type::place_maps_type)
            (trans.place_map());
          WWEAVE(transition::connect_read, ::xml::parse::type::connections_type)
            (trans.read());
          WWEAVE(transition::connect_in, ::xml::parse::type::connections_type)
            (trans.in());
          WWEAVE(transition::connect_out, ::xml::parse::type::connections_type)
            (trans.out());
          WWEAVE(transition::condition, ::xml::parse::type::conditions_type)
            (trans.cond);
          WWEAVEE(transition::close) ();
        }
      } // namespace detail

      // ******************************************************************* //

      StructureView::StructureView (const QString & load, QWidget* parent)
        : QTreeView (parent)
        , _model (new QStandardItemModel(this))
        , _root (_model->invisibleRootItem())
      {
        setModel (_model);

        setFrameShape(QFrame::StyledPanel);
        setFrameShadow(QFrame::Sunken);
        setDragDropMode(QAbstractItemView::DragOnly);

        header()->setVisible(false);

        if (load.length() > 0)
          {
            fromFile (load.toStdString());
          }
      }

      void StructureView::fromFile (const std::string & input)
      {
        ::xml::parse::state::type state;

        state.set_input (input);

        const ::xml::parse::type::function_type fun
          (::xml::parse::just_parse (state, input));

        from (fun);
      }

      void StructureView::from (const ::xml::parse::type::function_type & fun)
      {
        STATE w (_root);

        detail::from_function<STATE> (w, fun);
      }
    }
  }
}

#undef IT
#undef FUN
#undef FUN_IT
#undef SIG
#undef SIG_IT
