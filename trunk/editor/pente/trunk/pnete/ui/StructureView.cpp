#include "StructureView.hpp"

#include <QStandardItem>
#include <QStandardItemModel>
#include <QWidget>
#include <QHeaderView>
#include <QString>

#include <xml/parse/types.hpp>
#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <fhg/util/maybe.hpp>

#include <boost/variant.hpp>

#include <sstream>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace detail
      {
        void from_fun (QStandardItem * root
                      , const ::xml::parse::type::function_type * fun
                      );

        namespace visitor
        {
          template<typename NET>
          class from : public boost::static_visitor<void>
          {
          private:
            QStandardItem * root_item;

          public:
            from (QStandardItem * root)
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
                  expressions_item->appendRow
                    (new QStandardItem (QString ((*e).c_str())));
                }

              root_item->appendRow (expressions_item);
            }

            void operator () (const ::xml::parse::type::mod_type & mod) const
            {
              QStandardItem* mod_item (new QStandardItem ("module"));

              std::ostringstream oss;

              oss << mod.name << "->" << mod.function;

              QStandardItem* mod_entry_item
                (new QStandardItem (QString(oss.str().c_str())));

              mod_item->appendRow (mod_entry_item);

              root_item->appendRow (mod_item);
            }

            void operator () (const NET & net) const
            {
              QStandardItem* net_item (new QStandardItem ("net"));

              QStandardItem* places_item (new QStandardItem ("places"));

              for (typename NET::places_type::const_iterator
                     place (net.places().begin()), end (net.places().end())
                  ; place != end
                  ; ++place
                  )
                {
                  std::ostringstream oss;

                  oss << place->name << " :: " << place->type;

                  places_item->appendRow
                    (new QStandardItem (QString (oss.str().c_str())));
                }

              net_item->appendRow (places_item);

              QStandardItem* transitions_item
                (new QStandardItem ("transition"));

              for (typename NET::transitions_type::const_iterator
                     transition (net.transitions().begin())
                  , end (net.transitions().end())
                  ; transition != end
                  ; ++transition
                  )
                {
                  // WORK HERE: call from (transitions_item, *transition);
                }

              net_item->appendRow (transitions_item);

              root_item->appendRow (net_item);
            }
          };
        }

        void from_fun ( QStandardItem * root
                      , const ::xml::parse::type::function_type * fun
                      )
        {
          const QString name ( fun->name.isJust ()
                             ? QString ((*fun->name).c_str())
                             : "<fun without a name>"
                             );

          QStandardItem* fun_item (new QStandardItem (name));

          root->appendRow (fun_item);

          QStandardItem* conditions_item (new QStandardItem ("condition"));

          for (::xml::parse::type::conditions_type::const_iterator
                 cond (fun->cond.begin()), end (fun->cond.end())
              ; cond != end
              ; ++cond
              )
            {
              conditions_item->appendRow
                (new QStandardItem (QString ((*cond).c_str())));
            }

          fun_item->appendRow (conditions_item);

          QStandardItem* structs_item (new QStandardItem ("structs"));

          for (::xml::parse::type::structs_type::const_iterator
                 s (fun->structs.begin()), end (fun->structs.end())
              ; s != end
              ; ++s
              )
            {
              structs_item->appendRow
                (new QStandardItem (QString (s->name.c_str())));
            }

          fun_item->appendRow (structs_item);

          QStandardItem* ports_item (new QStandardItem ("port"));
          QStandardItem* ports_in_item (new QStandardItem ("in"));
          QStandardItem* ports_out_item (new QStandardItem ("out"));

          for (::xml::parse::type::ports_type::const_iterator
                 port (fun->in().begin()), end (fun->in().end())
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
                 port (fun->out().begin()), end (fun->out().end())
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

          boost::apply_visitor
            (visitor::from< ::xml::parse::type::net_type> (fun_item), fun->f);
        }
      }

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
          (::xml::parse::frontend (state, input));

        from (&fun);
      }

      void StructureView::from (const ::xml::parse::type::function_type * fun)
      {
        detail::from_fun (_root, fun);
      }
    }
  }
}
