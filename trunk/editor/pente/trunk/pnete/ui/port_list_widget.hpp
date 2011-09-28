// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_PORT_LIST_WIDGET_HPP
#define _FHG_PNETE_UI_PORT_LIST_WIDGET_HPP 1

#include <QObject>
#include <QTableView>

#include <pnete/data/proxy.hpp>

class QWidget;
class QStringList;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      //! \todo use tiberiu's type selections combobox
      //! \todo make this nicer
      //! \todo add edit facilities
      class port_list_widget : public QTableView
      {
        Q_OBJECT;

      public:
        explicit port_list_widget ( data::proxy::xml_type::ports_type& ports
                                  , const QStringList& list_types
                                  , QWidget* parent = NULL
                                  );

      private:
        data::proxy::xml_type::ports_type& _ports;
      };
    }
  }
}

#endif
