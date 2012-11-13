// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/port_list_widget.hpp>

#include <QHeaderView>
#include <QStandardItem>
#include <QStandardItemModel>

#include <pnete/ui/ComboItemDelegate.hpp>

#include <boost/foreach.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      port_list_widget::port_list_widget
        ( ::xml::parse::type::function_type::ports_type& ports
        , const QStringList& types
        , QWidget* parent
        )
          : QTableView (parent)
          , _ports (ports)
      {
        QStandardItemModel* model (new QStandardItemModel (0, 2, this));

        model->setColumnCount (2);

        setItemDelegateForColumn (1, new ComboBoxItemDelegate (types, this));

        model->setHorizontalHeaderItem (0, new QStandardItem ("Name"));
        model->setHorizontalHeaderItem (1, new QStandardItem ("Type"));

        verticalHeader()->hide();

        int row (0);

        BOOST_FOREACH ( const ::xml::parse::type::port_type& port
                      , _ports.values()
                      )
        {
          model->setItem
            (row, 0, new QStandardItem (QString::fromStdString(port.name())));
          model->setItem
            (row, 1, new QStandardItem (QString::fromStdString(port.type)));
          ++row;
        }

        resizeRowsToContents();
        resizeColumnsToContents();

    	setModel (model);

        setSelectionMode (QAbstractItemView::NoSelection);
      }
    }
  }
}
