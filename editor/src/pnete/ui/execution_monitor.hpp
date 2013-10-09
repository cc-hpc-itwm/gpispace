#ifndef FHG_PNETE_UI_EXECUTION_MONITOR_HPP
#define FHG_PNETE_UI_EXECUTION_MONITOR_HPP

#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class execution_monitor : public QWidget
      {
        Q_OBJECT

      public:
        execution_monitor (unsigned short port, QWidget* parent = NULL);
      };
    }
  }
}

#endif
