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
        namespace action
        {
          class type : public QAction
          {
            Q_OBJECT;

          private:
            graph::base_item* _x;

          public:
            type (graph::base_item*, QObject* = NULL);
            type (graph::base_item*, const QString&, QObject* = NULL);
            type (graph::base_item*, const QIcon&, const QString&, QObject* = NULL);

          private slots:
            void slot_triggered ();

          signals:
            void signal_triggered (graph::base_item*);

          private:
            void re_connect();
          };
        }
      }
    }
  }
}

#endif
