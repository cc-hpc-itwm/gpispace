// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_PORT_LIST_WIDGET_HPP
#define _FHG_PNETE_UI_PORT_LIST_WIDGET_HPP 1

#include <QObject>
#include <QTableWidget>

class QWidget;

#include <xml/parse/type/function.hpp> // ports_type..

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      //! \todo make this nicer
      //! \todo add edit facilities
      //! \todo adjust column sizes automatically
      class port_list_widget : public QTableView
      {
        Q_OBJECT;

      public:
        explicit port_list_widget ( ::xml::parse::type::function_type::ports_type& ports
                                  , const QStringList& list_types
                                  , QWidget* parent = NULL
                                  );

      private:
        ::xml::parse::type::function_type::ports_type& _ports;
      };
    }
  }
}

#endif
