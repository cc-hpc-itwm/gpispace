// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/port_list_widget.hpp>

#include <QHeaderView>
#include <QStandardItem>
#include <QStandardItemModel>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      port_list_widget::port_list_widget
        ( data::proxy::xml_type::ports_type& ports
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

        typedef data::proxy::xml_type::ports_type::iterator iterator;

        int row (0);

        for ( iterator port (_ports.begin()), end (_ports.end())
            ; port != end
            ; ++port, ++row
            )
        {
          model->setItem
            (row, 0, new QStandardItem (QString::fromStdString(port->name)));
          model->setItem
            (row, 1, new QStandardItem (QString::fromStdString(port->type)));
        }

        resizeRowsToContents(); // Adjust the row height.
        resizeColumnsToContents(); // Adjust the column width.

    	setModel (model);
      }
    }
  }
}
