// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <util/qt/no_undoredo_lineedit.hpp>

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
