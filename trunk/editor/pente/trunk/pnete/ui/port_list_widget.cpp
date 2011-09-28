// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/port_list_widget.hpp>

#include <QStandardItem>
#include <QStandardItemModel>
#include <QList>
#include <QStringList>
#include <QHeaderView>

#include <pnete/ui/ComboItemDelegate.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      port_list_widget::port_list_widget ( data::proxy::xml_type::ports_type& ps
                                         , const QStringList& list_types
                                         , QWidget* parent
                                         )
        : QTableView (parent)
        , _ports (ps)
      {
        QStandardItemModel* model (new QStandardItemModel (0, 2, this));

        QStringList headers;
        headers.push_back ("Name");
        headers.push_back ("Type");
        model->setHorizontalHeaderLabels (headers);

        setItemDelegateForColumn
          (1, (new ComboBoxItemDelegate (list_types, this)));

        verticalHeader()->hide();

        QList<QStandardItem *> row_items;

        typedef data::proxy::xml_type::ports_type::iterator iterator;
        for ( iterator port (_ports.begin()), end (_ports.end())
            ; port != end
            ; ++port
            )
        {
          row_items << new QStandardItem (QString::fromStdString (port->name))
                    << new QStandardItem (QString::fromStdString (port->type));
          model->appendRow (row_items);
          row_items.clear();
        }

        resizeRowsToContents();
        resizeColumnsToContents();

    	setModel (model);
      }
    }
  }
}
