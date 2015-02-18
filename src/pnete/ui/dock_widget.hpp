// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <pnete/ui/dock_widget.fwd.hpp>

#include <QDockWidget>

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
