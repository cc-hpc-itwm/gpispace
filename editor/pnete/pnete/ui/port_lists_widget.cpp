// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/port_lists_widget.hpp>

#include <pnete/data/handle/function.hpp>
#include <pnete/ui/ComboItemDelegate.hpp>

#include <xml/parse/type/function.hpp>

#include <boost/foreach.hpp>

#include <QGroupBox>
#include <QHeaderView>
#include <QObject>
#include <QSplitter>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTableWidget>
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
        QGroupBox* port_list_box
          ( const we::type::PortDirection& which
          , const data::handle::function& function
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

          //! \todo Should use handles and special items to allow
          //! editing. Also, this should be weaved in, not pulled.
          BOOST_FOREACH ( const ::xml::parse::type::port_type& port
                        , function.get().ports().values()
                        )
          {
            if (port.direction() == which)
            {
              model->setItem
                (row, 0, new QStandardItem (QString::fromStdString(port.name())));
              model->setItem
                (row, 1, new QStandardItem (QString::fromStdString(port.type)));
              ++row;
            }
          }

          table->resizeRowsToContents();
          table->resizeColumnsToContents();

          table->setModel (model);

          table->setSelectionMode (QAbstractItemView::NoSelection);


          QGroupBox* group_box
            ( new QGroupBox ( QObject::tr ( which == we::type::PORT_IN
                                          ? "in_ports_header"
                                          : ( which == we::type::PORT_OUT
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
        ( const data::handle::function& function
        , const QStringList& types
        , QWidget* parent
        )
          : QWidget (parent)
      {
        QSplitter* splitter (new QSplitter (Qt::Vertical));

        splitter->addWidget
          (port_list_box (we::type::PORT_IN, function, types));
        splitter->addWidget
          (port_list_box (we::type::PORT_OUT, function, types));
        splitter->addWidget
          (port_list_box (we::type::PORT_TUNNEL, function, types));

        QVBoxLayout* vbox (new QVBoxLayout());
        vbox->addWidget (splitter);
        vbox->setContentsMargins (0, 0, 0, 0);
        setLayout(vbox);
      }
    }
  }
}
