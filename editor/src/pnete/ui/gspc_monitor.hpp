// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_GSPC_MONITOR_HPP
#define FHG_PNETE_UI_GSPC_MONITOR_HPP

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
        gspc_monitor (QString host, int port, QWidget* parent = NULL);
      };
    }
  }
}

#endif
