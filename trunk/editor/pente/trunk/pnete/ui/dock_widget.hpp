// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_DOCK_WIDGET_HPP
#define _PNETE_UI_DOCK_WIDGET_HPP 1

#include <QObject>
#include <QDockWidget>

class QObject;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class dock_widget : public QDockWidget
      {
        Q_OBJECT

      public:
        dock_widget (const QString& title = "");
      };
    }
  }
}

#endif
