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
    namespace ui
    {
//       namespace tv
//       {
//         class State
//         {
//         public:
//           State (QStandardItem* root) : _root (root) {}

//           template<typename T> State append (const T & x) const
//           {
//             QStandardItem* x_item (new QStandardItem (x));
//             x_item->setEditable (false);
//             _root->appendRow (x_item);
//             return State (x_item);
//           }
//           template<> State append<std::string> (const std::string & str) const
//           {
//             return append (QString (str.c_str()));
//           }
//           template<> State append<boost::format> (const boost::format & f) const
//           {
//             return append (f.str());
//           }
//         private:
//           QStandardItem* _root;
//         };
//       }

      namespace util
      {
        template<typename T>
        static inline QStandardItem*
        append (QStandardItem* root, const T & x)
        {
          QStandardItem* x_item (new QStandardItem (x));
          x_item->setEditable (false);

          root->appendRow (x_item);

          return x_item;
        }

        template<>
        QStandardItem*
        append<std::string> (QStandardItem* root, const std::string & str)
        {
          return append (root, QString (str.c_str()));
        }

        template<>
        QStandardItem*
        append<boost::format> (QStandardItem* root, const boost::format & f)
        {
          return append (root, f.str());
        }
      } // namespace util

      namespace detail
      {
        template<typename State, typename IT>
        static inline void from_xs
        ( State root
        , const QString & header
        , IT pos
        , const IT & end
        , void (*fun) ( State
                      , const typename std::iterator_traits<IT>::value_type &
                      )
        )
        {
          if (pos != end)
            {
              State item (util::append (root, header));

              while (pos != end)
                {
                  fun (item, *pos);

                  ++pos;
                }
            }
        }

        template<typename State, typename Coll>
        static inline void from_xs
        ( State root
        , const std::string & header
        , const Coll & coll
        , void (*fun) ( State
                      , const typename std::iterator_traits<typename Coll::const_iterator>::value_type &
                      )
        )
        {
          return from_xs ( root
                         , QString (header.c_str())
                         , coll.begin()
                         , coll.end()
                         , fun
                         );
        }

#if (  defined IT      \
    || defined FUNT    \
    || defined FUN     \
    || defined FUN_IT  \
    || defined SIG     \
    || defined SIG_IT  \
    )
#error "Macro already defined"
#endif

#define IT(type) type::const_iterator::value_type

#define FUN(name,type,var) \
        template<typename State> \
        static void from_ ## name (State state, const type & var)
#define FUN_IT(name,type,var) FUN(name,IT(type),var)
#define SIG(name,type) FUN(name,type,)
#define SIG_IT(name,type) SIG(name,IT(type))

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
              util::append (_state, boost::format("%s") % t);
            }

            void operator () (const ::signature::structured_t & map) const
            {
              for ( ::signature::structured_t::const_iterator field (map.begin())
                  ; field != map.end()
                  ; ++field
                  )
                {
                  boost::apply_visitor
                    ( from_token (util::append (_state, field->first))
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
              from_xs ( _state
                      , "expression"
                      , exp.expressions
                      , from_expression_sequence
                      );
            }

            void operator () (const ::xml::parse::type::mod_type & mod) const
            {
              State mod_item
                ( util::append
                  ( util::append (_state, "module")
                  , boost::format ("%s -> %s")
                  % mod.name
                  % ::xml::parse::type::dump::dump_fun (mod)
                  )
                );

              from_xs (mod_item, "cinclude", mod.cincludes, from_cinclude);
              from_xs (mod_item, "link", mod.links, from_link);

              if (mod.code)
                {
                  util::append (util::append (mod_item, "code"), *mod.code);
                }
            }

            void operator () (const ::xml::parse::type::net_type & net) const
            {
              State net_item (util::append (_state, "net"));

              from_properties (net_item, net.prop);
              from_structs (net_item, net.structs);
              from_xs (net_item, "template", net.templates(), from_function);
              from_xs (net_item, "specialize", net.specializes(), from_specialize);
              from_xs (net_item, "function", net.functions(), from_function);
              from_xs (net_item, "place", net.places(), from_place);
              from_xs (net_item, "transition", net.transitions(), from_transition);
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
              util::append (_state, boost::format("use: %s") % use.name);
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
              util::append (_state, boost::format("%s :: %s") % _name % t);
            }

            void operator () (const ::signature::structured_t & map) const
            {
              from_xs (_state, _name, map, from_structured);
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
              util::append (_state, boost::format("%s: %s") % _key % v);
            }

            void operator () (const ::we::type::property::type & prop) const
            {
              from_properties (util::append (_state, _key), prop);
            }
          };
        } // namespace visitor

        FUN_IT(property, ::we::type::property::map_type, p)
        {
          boost::apply_visitor
            ( visitor::from_property<State> (state, p.first)
            , p.second
            );
        }

        FUN(properties, ::we::type::property::type, prop)
        {
          from_xs (state, "property", prop.get_map(), from_property);
        }

        FUN_IT(structured, ::signature::structured_t, s)
        {
          boost::apply_visitor
            ( visitor::from_signature<State> (s.first, state)
            , s.second
            );
        }

        FUN_IT(struct, ::xml::parse::type::structs_type, s)
        {
          boost::apply_visitor
            ( visitor::from_signature<State> (s.name, state)
            , s.sig
            );
        }

        FUN_IT(port, ::xml::parse::type::ports_type, port)
        {
          QStandardItem* port_item
            ( util::append ( state
                           , boost::format("%s :: %s") % port.name % port.type
                           )
            );

          if (port.place)
            {
              util::append ( port_item
                           , boost::format("place: %s") % *port.place
                           );
            }

          from_properties (port_item, port.prop);
        }

        FUN_IT(cinclude, ::xml::parse::type::cincludes_type, cinclude)
        {
          util::append (state, cinclude);
        }

        FUN_IT(link, ::xml::parse::type::links_type, link)
        {
          util::append (state, link);
        }

        FUN(structs, ::xml::parse::type::structs_type, structs)
        {
          from_xs (state, "struct", structs, from_struct);
        }

        FUN(expression_sequence, std::string, lines)
        {
          const char b (';');
          std::string::const_iterator pos (lines.begin());
          const std::string::const_iterator end (lines.end());

          std::string line;

          while (pos != end)
            {
              switch (*pos)
                {
                case b:
                  util::append (state, line);
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
              util::append (state, line);
            }
        }

        FUN_IT(type_get, ::xml::parse::type::type_get_type, tg)
        {
          util::append (state, tg);
        }

        FUN_IT(type_map, ::xml::parse::type::type_map_type, tm)
        {
          util::append (state, boost::format("%s => %s") % tm.first % tm.second);
        }

        FUN_IT(specialize, ::xml::parse::type::net_type::specializes_type, spec)
        {
          State spec_item
            ( util::append ( state
                           , boost::format("%s use %s") % spec.name % spec.use
                           )
            );

          from_xs (spec_item, "type_map", spec.type_map, from_type_map);
          from_xs (spec_item, "type_get", spec.type_get, from_type_get);
        }


        FUN(conditions, ::xml::parse::type::conditions_type, cs)
        {
          from_xs (state, "condition", cs, from_expression_sequence);
        }

        FUN_IT(function, ::xml::parse::type::net_type::functions_type, fun)
        {
          State fun_item (util::append ( state
                                       , fun.name
                                       ? QString ((*fun.name).c_str())
                                       : "<<function>>"
                                       )
                         );

          if (fun.internal)
            {
              util::append ( fun_item
                           , boost::format ("internal: %s")
                           % (*fun.internal ? "true" : "false")
                           );
            }

          from_xs (fun_item, "require", fun.requirements, from_require);

          from_properties (fun_item, fun.prop);
          from_structs (fun_item, fun.structs);

          from_xs (fun_item, "in", fun.in(), from_port);
          from_xs (fun_item, "out", fun.out(), from_port);

          boost::apply_visitor
            ( visitor::from_net_type<State> (fun_item)
            , fun.f
            );

          from_conditions (fun_item, fun.cond);
        }

        FUN_IT(place, ::xml::parse::type::places_type, place)
        {
          State place_item
            ( util::append
              ( state
              , boost::format("%s :: %s") % place.name % place.type
              )
            );

          if (place.is_virtual)
            {
              util::append
                ( place_item
                , boost::format ("virtual: %s")
                % (*place.is_virtual ? "true" : "false")
                );
            }

          if (place.capacity)
            {
              util::append
                ( place_item
                , boost::format ("capacity: %i") % *place.capacity
                );
            }

          from_properties (place_item, place.prop);

          for ( ::xml::parse::type::tokens_type::const_iterator
                  tok (place.tokens.begin())
              ; tok != place.tokens.end()
              ; ++tok
              )
            {
              boost::apply_visitor
                ( visitor::from_token<State>
                  (util::append (place_item, "token"))
                , *tok
                );
            }
        }

        FUN_IT(place_map, ::xml::parse::type::place_maps_type, pm)
        {
          State pm_item ( util::append
                          ( state
                          , boost::format ("virtual: %s <-> real: %s")
                          % pm.place_virtual
                          % pm.place_real
                          )
                        );

          from_properties (pm_item, pm.prop);
        }

        FUN_IT(connection, ::xml::parse::type::connections_type, connect)
        {
          util::append ( state
                       , boost::format ("port: %s -> place: %s")
                       % connect.place
                       % connect.port
                       );
        }

        FUN_IT(require, ::xml::parse::type::requirements_type, req)
        {
          util::append ( state
                       , boost::format ("%s: %s")
                       % req.first
                       % (req.second ? "true" : "false")
                       );
        }

        FUN_IT(transition, ::xml::parse::type::net_type::transitions_type, trans)
        {
          State transition_item (util::append (state, trans.name));

          if (trans.priority)
            {
              util::append ( transition_item
                           , boost::format ("priority: %i") % *trans.priority
                           );
            }

          if (trans.internal)
            {
              util::append ( transition_item
                           , boost::format ("internal: %i")
                           % (*trans.internal ? "true" : "false")
                           );
            }

          from_properties (transition_item, trans.prop);

          from_structs (transition_item, trans.structs);

          boost::apply_visitor
            ( visitor::from_function_type<QStandardItem *> (transition_item)
            , trans.f
            );

          from_xs (transition_item, "place-map", trans.place_map(), from_place_map<QStandardItem *>);

          from_xs (transition_item, "connect-read", trans.read(), from_connection);
          from_xs (transition_item, "connect-in", trans.in(), from_connection);
          from_xs (transition_item, "connect-out", trans.out(), from_connection);

          from_conditions (transition_item, trans.cond);
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
        detail::from_function<QStandardItem*> (_root, fun);
      }
    }
  }
}

#undef IT
#undef FUN
#undef FUN_IT
#undef SIG
#undef SIG_IT
