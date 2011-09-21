// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/port_list_widget.hpp>

#include <QStringList>

#include <QTableWidgetItem>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      port_list_widget::port_list_widget
        ( data::proxy::xml_type::ports_type& in
        , data::proxy::xml_type::ports_type& out
        , QWidget* parent
        )
          : QTableWidget (parent)
          , _in (in)
          , _out (out)
      {
        setColumnCount (3);
        setRowCount (_in.size() + _out.size());

        QStringList headers;

        headers.push_back ("direction");
        headers.push_back ("name");
        headers.push_back ("type");

        setHorizontalHeaderLabels (headers);

        int row (0);

        set_rows (&row, "in", _in);
        set_rows (&row, "out", _out);
      }

      void port_list_widget::set_rows ( int * row
                                      , const QString& direction
                                      , data::proxy::xml_type::ports_type& ports
                                      )
      {
        typedef data::proxy::xml_type::ports_type::iterator iterator;

        for ( iterator port (ports.begin()), end (ports.end())
            ; port != end
            ; ++port, ++*row
            )
          {
            set_row (*row, direction, *port);
          }
      }

      void port_list_widget::set_row ( const int row
                                     , const QString& direction
                                     , data::proxy::xml_type::port_type& port
                                     )
      {
        setItem (row, 0, new QTableWidgetItem (direction));
        setItem (row, 1, new QTableWidgetItem (QString(port.name.c_str())));
        setItem (row, 2, new QTableWidgetItem (QString(port.type.c_str())));
      }
    }
  }
}
