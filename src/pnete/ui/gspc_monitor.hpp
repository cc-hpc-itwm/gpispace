#pragma once

#include <QSplitter>
#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class gspc_monitor : public QSplitter
      {
        Q_OBJECT

      public:
        gspc_monitor (QString host, int port, QWidget* parent = nullptr);
      };
    }
  }
}
