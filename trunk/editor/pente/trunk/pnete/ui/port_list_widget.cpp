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
      port_list_widget::port_list_widget
        ( data::proxy::xml_type::ports_type& ports
        , QWidget* parent
        )
          : QTableWidget (parent)
          , _ports (ports)
      {
        setColumnCount (2);
        setRowCount (_ports.size());

        QStringList headers;

        headers.push_back ("name");
        headers.push_back ("type");

        setHorizontalHeaderLabels (headers);
        verticalHeader()->hide();

        int row (0);

        typedef data::proxy::xml_type::ports_type::iterator iterator;

        for ( iterator port (_ports.begin()), end (_ports.end())
            ; port != end
            ; ++port, ++row
            )
          {
            setItem (row, 0, new QTableWidgetItem (QString(port->name.c_str())));
            setItem (row, 1, new QTableWidgetItem (QString(port->type.c_str())));
          }
      }
    }
  }
}
