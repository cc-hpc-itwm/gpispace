// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_PORT_LIST_WIDGET_HPP
#define _FHG_PNETE_UI_PORT_LIST_WIDGET_HPP 1

#include <QObject>
#include <QTableWidget>

#include <pnete/data/proxy.hpp>

class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      //! \todo use tiberiu's type selections combobox
      //! \todo implies to use QTableView
      //! \todo make this nicer
      //! \todo add edit facilities
      class port_list_widget : public QTableWidget
      {
        Q_OBJECT;

      public:
        explicit port_list_widget ( data::proxy::xml_type::ports_type& in
                                  , data::proxy::xml_type::ports_type& out
                                  , QWidget* parent = NULL
                                  );

      private:
        data::proxy::xml_type::ports_type& _in;
        data::proxy::xml_type::ports_type& _out;

        void set_row ( const int row
                     , const QString& direction
                     , data::proxy::xml_type::port_type & port
                     );

        void set_rows ( int* row
                      , const QString& direction
                      , data::proxy::xml_type::ports_type& ports
                      );
      };
    }
  }
}

#endif
