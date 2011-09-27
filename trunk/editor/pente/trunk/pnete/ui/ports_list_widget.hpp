// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_PORTS_LIST_WIDGET_HPP
#define _FHG_PNETE_UI_PORTS_LIST_WIDGET_HPP 1

#include <QObject>
#include <QWidget>

#include <pnete/data/proxy.hpp>
#include <pnete/ui/port_list_widget.hpp>

class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class ports_list_widget : public QWidget
      {
        Q_OBJECT;

      public:
        explicit ports_list_widget ( data::proxy::xml_type::ports_type& in
                                   , data::proxy::xml_type::ports_type& out
                                   , QWidget* parent = NULL
                                   );

      private:
        QStringList list_combo_types();
        void init_test_data(data::proxy::xml_type::ports_type&);

        data::proxy::xml_type::ports_type& _in;
        data::proxy::xml_type::ports_type& _out;
      };
    }
  }
}

#endif
