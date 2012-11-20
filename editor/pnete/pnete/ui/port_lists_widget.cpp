// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/port_lists_widget.hpp>

#include <pnete/ui/ComboItemDelegate.hpp>

#include <boost/foreach.hpp>

#include <QObject>
#include <QTableWidget>
#include <xml/parse/type/function.hpp> // ports_type..
#include <QHeaderView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QGroupBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace
      {
        enum which_ports
        {
          in,
          out,
          tunnel,
        };

        QGroupBox* port_list_box
          ( const which_ports& which
          , const ::xml::parse::id::ref::function& function_id
          , const QStringList& types
          )
        {
          //! \todo make this nicer
          //! \todo add edit facilities
          //! \todo adjust column sizes automatically
          QTableView* table (new QTableView);

          QStandardItemModel* model (new QStandardItemModel (0, 2, table));

          model->setColumnCount (2);

          table->setItemDelegateForColumn
            (1, new ComboBoxItemDelegate (types, table));

          model->setHorizontalHeaderItem (0, new QStandardItem ("Name"));
          model->setHorizontalHeaderItem (1, new QStandardItem ("Type"));

          table->verticalHeader()->hide();

          int row (0);

          const ::xml::parse::type::function_type& function (function_id.get());

          BOOST_FOREACH ( const ::xml::parse::type::port_type& port
                        , ( which == in
                          ? function.in()
                          : ( which == out
                            ? function.out()
                            : function.tunnel()
                            )
                          ).values()
                        )
          {
            model->setItem
              (row, 0, new QStandardItem (QString::fromStdString(port.name())));
            model->setItem
              (row, 1, new QStandardItem (QString::fromStdString(port.type)));
            ++row;
          }

          table->resizeRowsToContents();
          table->resizeColumnsToContents();

          table->setModel (model);

          table->setSelectionMode (QAbstractItemView::NoSelection);


          QGroupBox* group_box
            ( new QGroupBox ( QObject::tr ( which == in
                                          ? "in_ports_header"
                                          : ( which == out
                                            ? "out_ports_header"
                                            : "tunnel_ports_header"
                                            )
                                          )
                            )
            );
          QVBoxLayout* vbox (new QVBoxLayout (group_box));
          vbox->addWidget (table);
          group_box->setLayout (vbox);
          return group_box;
        }
      }

      port_lists_widget::port_lists_widget
        ( const ::xml::parse::id::ref::function& function
        , const QStringList& types
        , QWidget* parent
        )
          : QWidget (parent)
      {
        QSplitter* splitter (new QSplitter (Qt::Vertical));

        splitter->addWidget (port_list_box (in, function, types));
        splitter->addWidget (port_list_box (out, function, types));
        splitter->addWidget (port_list_box (tunnel, function, types));

        QVBoxLayout* vbox (new QVBoxLayout());
        vbox->addWidget (splitter);
        vbox->setContentsMargins (0, 0, 0, 0);
        setLayout(vbox);
      }
    }
  }
}
