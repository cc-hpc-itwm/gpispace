// bernd.loerwald@itwm.fraunhofer.de

#include "no_undoredo_lineedit.hpp"

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPointer>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      no_undoredo_lineedit::no_undoredo_lineedit (QWidget* parent)
        : QLineEdit (parent)
      { }

      no_undoredo_lineedit::no_undoredo_lineedit (const QString& c, QWidget* p)
        : QLineEdit (c, p)
      { }

      void no_undoredo_lineedit::contextMenuEvent (QContextMenuEvent* event)
      {
        QPointer<QMenu> menu (createStandardContextMenu());

        QList<QAction*> actions (menu->actions());
        menu->removeAction (actions.takeFirst()); // undo
        menu->removeAction (actions.takeFirst()); // redo
        menu->removeAction (actions.takeFirst()); // separator

        menu->exec (event->globalPos());
        delete menu;
      }

      void no_undoredo_lineedit::keyPressEvent (QKeyEvent* event)
      {
#ifndef QT_NO_SHORTCUT
        if (event == QKeySequence::Undo || event == QKeySequence::Redo)
        {
          return QWidget::keyPressEvent (event);
        }
#endif
        return QLineEdit::keyPressEvent (event);
      }

      bool no_undoredo_lineedit::event(QEvent* e)
      {
#ifndef QT_NO_SHORTCUT
        if (e->type() == QEvent::ShortcutOverride)
        {
          QKeyEvent* ke (static_cast<QKeyEvent*> (e));
          if (ke == QKeySequence::Undo || ke == QKeySequence::Redo)
          {
            return QWidget::event (e);
          }
        }
#endif
        return QLineEdit::event (e);
      }
    }
  }
}
