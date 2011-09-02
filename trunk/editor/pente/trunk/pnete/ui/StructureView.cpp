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

#include <sstream>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace util
      {
        void add_lines ( QStandardItem * root
                       , const std::string & lines
                       , const char & b = ';'
                       )
        {
          std::string line;

          std::string::const_iterator pos (line.begin());
          const std::string::const_iterator end (line.end());

          while (pos != end)
            {
              if (*pos == b)
                {
                  root->appendRow (new QStandardItem (QString (line.c_str())));

                  line.clear();

                  ++pos;

                  while (pos != end && (isspace (*pos) || *pos == b))
                    {
                      ++pos;
                    }
                }
              else
                {
                  line += *pos;
                  ++pos;
                  break;
                }
            }
        }
      } // namespace util

      namespace detail
      {
        void from_fun ( QStandardItem * root
                      , const ::xml::parse::type::function_type & fun
                      );
        void from_transition ( QStandardItem * root
                             , const ::xml::parse::type::transition_type & trans
                             );
        void from_structs ( QStandardItem * root
                          , const ::xml::parse::type::structs_type & structs
                          );
        void from_conditions ( QStandardItem * root
                             , const ::xml::parse::type::conditions_type & cs
                             );

        namespace visitor
        {
          class from_token : public boost::static_visitor<void>
          {
          private:
            const std::string & name;
            QStandardItem* root_item;

          public:
            from_token (const std::string & _name, QStandardItem* root)
              : name (_name)
              , root_item (root)
            {}

            void operator () (const literal::type_name_t & t) const
            {
              std::ostringstream oss;

              oss << name << ": " << t;

              root_item->appendRow
                (new QStandardItem (QString (oss.str().c_str())));
            }

            void operator () (const signature::structured_t & map) const
            {
              for ( signature::structured_t::const_iterator field (map.begin())
                  ; field != map.end()
                  ; ++field
                  )
                {
                  QStandardItem* tok_item
                    (new QStandardItem (QString (name.c_str())));
                  root_item->appendRow (tok_item);

                  boost::apply_visitor ( from_token (field->first, tok_item)
                                       , field->second
                                       );
                }
            }
          };

          class from_net_type : public boost::static_visitor<void>
          {
          private:
            QStandardItem * root_item;

          public:
            from_net_type (QStandardItem * root)
              : root_item (root)
            {}

            void operator () (const ::xml::parse::type::expression_type & exp) const
            {
              QStandardItem* expressions_item (new QStandardItem ("expression"));

              for (::xml::parse::type::expressions_type::const_iterator
                     e (exp.expressions.begin()), end (exp.expressions.end())
                  ; e != end
                  ; ++e
                  )
                {
                  util::add_lines (expressions_item, *e);
                }

              root_item->appendRow (expressions_item);
            }

            void operator () (const ::xml::parse::type::mod_type & mod) const
            {
              QStandardItem* mod_item (new QStandardItem ("module"));

              std::ostringstream oss;

              oss << mod.name << " -> " << mod.function;

              QStandardItem* mod_entry_item
                (new QStandardItem (QString(oss.str().c_str())));

              mod_item->appendRow (mod_entry_item);

              root_item->appendRow (mod_item);
            }

            void operator () (const ::xml::parse::type::net_type & net) const
            {
              QStandardItem* net_item (new QStandardItem ("net"));
              root_item->appendRow (net_item);

              from_structs (net_item, net.structs);

              QStandardItem* templates_item (new QStandardItem ("template"));
              net_item->appendRow (templates_item);
              for (::xml::parse::type::net_type::templates_type::const_iterator
                     templ (net.templates().begin())
                  , end_templ (net.templates().end())
                  ; templ != end_templ
                  ; ++templ
                  )
                {
                  from_fun (templates_item, *templ);
                }

              QStandardItem* specializes_item (new QStandardItem ("specialize"));
              net_item->appendRow (specializes_item);
              for (::xml::parse::type::net_type::specializes_type::const_iterator
                     spec (net.specializes().begin())
                  , end_spec (net.specializes().end())
                  ; spec != end_spec
                  ; ++spec
                  )
                {
                  std::ostringstream oss_name;

                  oss_name << spec->name << " use " << spec->use;

                  QStandardItem* spec_item
                    (new QStandardItem (QString (oss_name.str().c_str())));
                  specializes_item->appendRow (spec_item);

                  QStandardItem* type_map_item
                    (new QStandardItem ("type_map"));
                  spec_item->appendRow (type_map_item);

                  for (::xml::parse::type::type_map_type::const_iterator
                         tm (spec->type_map.begin())
                      , end_tm (spec->type_map.end())
                      ; tm != end_tm
                      ; ++tm
                      )
                    {
                      std::ostringstream oss_type_map;

                      oss_type_map << tm->first << " => " << tm->second;

                      type_map_item->appendRow
                        (new QStandardItem (QString (oss_type_map.str().c_str())));
                    }

                  QStandardItem* type_get_item
                    (new QStandardItem ("type_get"));
                  spec_item->appendRow (type_get_item);

                  for (::xml::parse::type::type_get_type::const_iterator
                         tg (spec->type_get.begin())
                      , end_tg (spec->type_get.end())
                      ; tg != end_tg
                      ; ++tg
                      )
                    {
                      type_get_item->appendRow
                        (new QStandardItem (QString (tg->c_str())));
                    }
                }

              QStandardItem* functions_item (new QStandardItem ("function"));
              net_item->appendRow (functions_item);
              for (::xml::parse::type::net_type::functions_type::const_iterator
                     fun (net.functions().begin())
                  , end_fun (net.functions().end())
                  ; fun != end_fun
                  ; ++fun
                  )
                {
                  from_fun (functions_item, *fun);
                }

              QStandardItem* places_item (new QStandardItem ("place"));
              net_item->appendRow (places_item);

              for (::xml::parse::type::net_type::places_type::const_iterator
                     place (net.places().begin()), end (net.places().end())
                  ; place != end
                  ; ++place
                  )
                {
                  std::ostringstream oss;

                  oss << place->name << " :: " << place->type;

                  QStandardItem* place_item
                    (new QStandardItem (QString (oss.str().c_str())));
                  places_item->appendRow (place_item);

                  for ( ::xml::parse::type::tokens_type::const_iterator
                          tok (place->tokens.begin())
                      ; tok != place->tokens.end()
                      ; ++tok
                      )
                    {
                      boost::apply_visitor
                        ( visitor::from_token ("token", place_item)
                        , *tok
                        );
                    }
                }

              QStandardItem* transitions_item
                (new QStandardItem ("transition"));
              net_item->appendRow (transitions_item);

              for (::xml::parse::type::net_type::transitions_type::const_iterator
                     transition (net.transitions().begin())
                  , end (net.transitions().end())
                  ; transition != end
                  ; ++transition
                  )
                {
                  from_transition (transitions_item, *transition);
                }
            }
          };

          class from_function_type : public boost::static_visitor<void>
          {
          private:
            QStandardItem * root_item;

          public:
            from_function_type (QStandardItem* root)
              : root_item (root)
            {}

            void operator () (const ::xml::parse::type::use_type & use) const
            {
              std::ostringstream oss;

              oss << "use: " << use.name;

              root_item->appendRow
                (new QStandardItem (QString (oss.str().c_str())));
            }

            void operator () (const ::xml::parse::type::function_type & fun) const
            {
              from_fun (root_item, fun);
            }
          };

          class from_signature : public boost::static_visitor<void>
          {
          private:
            const std::string & name;
            QStandardItem * root_item;

          public:
            from_signature (const std::string & _name, QStandardItem * root)
              : name (_name)
              , root_item (root)
            {}

            void operator () (const literal::type_name_t & t) const
            {
              std::ostringstream oss;

              oss << name << " :: " << t;

              root_item->appendRow
                (new QStandardItem (QString (oss.str().c_str())));
            }

            void operator () (const signature::structured_t & map) const
            {
              QStandardItem* sub (new QStandardItem (QString(name.c_str())));
              root_item->appendRow (sub);

              for ( signature::structured_t::const_iterator field (map.begin())
                  ; field != map.end()
                  ; ++field
                  )
                {
                  boost::apply_visitor ( from_signature (field->first, sub)
                                       , field->second
                                       );
                }
            }
          };
        } // namespace visitor

        void from_structs ( QStandardItem * root
                          , const ::xml::parse::type::structs_type & structs
                          )
        {
          QStandardItem* structs_item (new QStandardItem ("struct"));
          root->appendRow (structs_item);

          typedef ::xml::parse::type::structs_type::const_iterator const_it;

          for (const_it s (structs.begin()), end (structs.end()); s != end; ++s)
            {
              boost::apply_visitor
                (visitor::from_signature (s->name, structs_item), s->sig);
            }
        }

        void from_conditions ( QStandardItem * root
                             , const ::xml::parse::type::conditions_type & cs
                             )
        {
          QStandardItem* conditions_item (new QStandardItem ("condition"));
          root->appendRow (conditions_item);

          for (::xml::parse::type::conditions_type::const_iterator
                 cond (cs.begin()), end (cs.end())
              ; cond != end
              ; ++cond
              )
            {
              util::add_lines (conditions_item, *cond);
            }
        }

        void from_fun ( QStandardItem * root
                      , const ::xml::parse::type::function_type & fun
                      )
        {
          const QString name ( fun.name
                             ? QString ((*fun.name).c_str())
                             : "<<function>>"
                             );

          QStandardItem* fun_item (new QStandardItem (name));

          root->appendRow (fun_item);

          from_conditions (fun_item, fun.cond);
          from_structs (fun_item, fun.structs);

          QStandardItem* ports_item (new QStandardItem ("port"));
          QStandardItem* ports_in_item (new QStandardItem ("in"));
          QStandardItem* ports_out_item (new QStandardItem ("out"));

          for (::xml::parse::type::ports_type::const_iterator
                 port (fun.in().begin()), end (fun.in().end())
              ; port != end
              ; ++port
              )
            {
              std::ostringstream oss;

              oss << port->name << " :: " << port->type;

              ports_in_item->appendRow
                (new QStandardItem (QString (oss.str().c_str())));
            }

          for (::xml::parse::type::ports_type::const_iterator
                 port (fun.out().begin()), end (fun.out().end())
              ; port != end
              ; ++port
              )
            {
              std::ostringstream oss;

              oss << port->name << " :: " << port->type;

              ports_out_item->appendRow
                (new QStandardItem (QString (oss.str().c_str())));
            }

          ports_item->appendRow (ports_in_item);
          ports_item->appendRow (ports_out_item);

          fun_item->appendRow (ports_item);

          boost::apply_visitor (visitor::from_net_type (fun_item), fun.f);
        }

        void from_transition ( QStandardItem * root
                             , const ::xml::parse::type::transition_type & trans
                             )
        {
          QStandardItem* transition_item
            (new QStandardItem (QString (trans.name.c_str())));
          root->appendRow (transition_item);

          from_structs (transition_item, trans.structs);
          from_conditions (transition_item, trans.cond);

          boost::apply_visitor
            (visitor::from_function_type (transition_item), trans.f);
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
        detail::from_fun (_root, fun);
      }
    }
  }
}
