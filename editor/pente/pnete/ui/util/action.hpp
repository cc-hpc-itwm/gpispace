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
      namespace graph { class item; }

      namespace util
      {
        namespace action
        {
          class type : public QAction
          {
            Q_OBJECT;

          private:
            graph::item* _x;

          public:
            type (graph::item*, QObject* = NULL);
            type (graph::item*, const QString&, QObject* = NULL);
            type (graph::item*, const QIcon&, const QString&, QObject* = NULL);

          private slots:
            void slot_triggered ();

          signals:
            void signal_triggered (graph::item*);

          private:
            void re_connect();
          };
        }
      }
    }
  }
}

#endif
