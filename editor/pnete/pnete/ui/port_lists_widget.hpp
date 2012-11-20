// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_PORT_LISTS_WIDGET_HPP
#define _FHG_PNETE_UI_PORT_LISTS_WIDGET_HPP 1

#include <xml/parse/id/types.hpp>

#include <QObject>
#include <QWidget>

class QStringList;
class QWidget;

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
        explicit port_lists_widget ( const ::xml::parse::id::ref::function&
                                   , const QStringList& types
                                   , QWidget* parent = NULL
                                   );
      };
    }
  }
}

#endif
