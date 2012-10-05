// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_UTIL_ACTION_HPP
#define _FHG_PNETE_UI_UTIL_ACTION_HPP 1

#include <QObject>
#include <QAction>

class QString;
class QIcon;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph { class base_item; }

      namespace util
      {
        //! \note An action forwarding QAction::triggered() to
        //! action::signal_triggered(graph::base_item*).
        class action : public QAction
        {
          Q_OBJECT;

        public:
          action (graph::base_item*, QObject* = NULL);
          action (graph::base_item*, const QString&, QObject* = NULL);
          action (graph::base_item*, const QIcon&, const QString&, QObject* = NULL);

        private slots:
          void slot_triggered ();

        signals:
          void signal_triggered (graph::base_item*);

        private:
          void re_connect();

          graph::base_item* _x;
        };
      }
    }
  }
}

#endif
