// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_PORT_LISTS_WIDGET_HPP
#define _FHG_PNETE_UI_PORT_LISTS_WIDGET_HPP 1

#include <QObject>
#include <QWidget>

class QStringList;
class QWidget;

#include <xml/parse/type/function.hpp> // ports_type..

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class port_lists_widget : public QWidget
      {
        Q_OBJECT;

      public:
        explicit port_lists_widget ( ::xml::parse::type::function_type::ports_type& in
                                   , ::xml::parse::type::function_type::ports_type& out
                                   , const QStringList& types
                                   , QWidget* parent = NULL
                                   );

      private:
        ::xml::parse::type::function_type::ports_type& _in;
        ::xml::parse::type::function_type::ports_type& _out;

        const QStringList& _types;
      };
    }
  }
}

#endif
