// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/port_list_widget.hpp>

#include <QStringList>
#include <QTableWidgetItem>
#include <QHeaderView>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      port_list_widget::port_list_widget( data::proxy::xml_type::ports_type& ports
    		  	  	  	  	  	  	  	  , const QStringList& list_types
    		  	  	  	  	  	  	  	  , QWidget* parent
        )
          : QTableView (parent)
      	  , model(0,2)
          , _ports (ports)
      {
        model.setColumnCount(2);

        QStringList headers;

        headers.push_back ("Name");
        headers.push_back ("Type");

        model.setHorizontalHeaderLabels (headers);

    	setModel(&model);

		ComboBoxItemDelegate* delegate = new ComboBoxItemDelegate(list_types, this);
		setItemDelegateForColumn(1, delegate);

		verticalHeader()->hide();

		resizeRowsToContents(); // Adjust the row height.
		resizeColumnsToContents(); // Adjust the column width.
		setColumnWidth( 0, 130 );
		setColumnWidth( 1, 140 );

        QList<QStandardItem *> arrItems;
		QStandardItem *itemName, *itemType;

		typedef data::proxy::xml_type::ports_type::iterator iterator;
	    int row (0);
        for ( iterator port (_ports.begin()), end (_ports.end())
            ; port != end
            ; ++port, ++row
            )
        {
        	itemName = new QStandardItem(QString::fromStdString(port->name));
            itemType = new QStandardItem(QString::fromStdString(port->type));
            arrItems<<itemName<<itemType;
            model.appendRow(arrItems);
            arrItems.clear();
        }
      }
    }
  }
}
