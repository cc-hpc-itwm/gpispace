#pragma once

#include <QSplitter>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class execution_monitor : public QSplitter
      {
        Q_OBJECT

      public:
        execution_monitor (unsigned short port, QWidget* parent = nullptr);
      };
    }
  }
}
