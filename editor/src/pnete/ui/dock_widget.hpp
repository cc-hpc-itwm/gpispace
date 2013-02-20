// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_DOCK_WIDGET_HPP
#define _PNETE_UI_DOCK_WIDGET_HPP 1

#include <pnete/ui/dock_widget.fwd.hpp>

#include <QDockWidget>

class Widget;

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
        explicit dock_widget (const QString& = "<<unnamed dock_widget>>");
        explicit dock_widget (const QString&, QWidget*);

      private:
        void init();
      };
    }
  }
}

#endif
