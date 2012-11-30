// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/util/action.hpp>

#include <iostream>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace util
      {
        action::action (graph::base_item* x, QObject* object)
          : QAction (object)
          , _x (x)
        {
          re_connect();
        }
        action::action (graph::base_item* x, const QString& text, QObject* parent)
          : QAction (text, parent)
          , _x (x)
        {
          re_connect();
        }
        action::action ( graph::base_item* x
                       , const QIcon& icon
                       , const QString& text
                       , QObject* parent
                       )
          : QAction (icon, text, parent)
          , _x (x)
        {
          re_connect();
        }

        void action::slot_triggered ()
        {
          emit signal_triggered (_x);
        }
        void action::re_connect()
        {
          connect (this, SIGNAL (triggered()), SLOT (slot_triggered()));
        }
      }
    }
  }
}
