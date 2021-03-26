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

#include <util-qt/compat.hpp>
#include <util/qt/treeview_with_delete.hpp>

#include <QKeyEvent>
#include <QList>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QSet>
#include <QVector>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      treeview_with_delete::treeview_with_delete (QWidget* parent)
        : QTreeView (parent)
      {}

      namespace
      {
        QSet<QModelIndex> indices_with_parent_in_set_removed (QSet<QModelIndex> set)
        {
          for ( QMutableSetIterator<QModelIndex> iter (set)
              ; iter.hasNext()
              ;
              )
          {
            QModelIndex index (iter.next());
            for ( QModelIndex parent (index.parent())
                ; parent.isValid()
                ; parent = parent.parent()
                )
            {
              if (set.contains (parent))
              {
                iter.remove();
                break;
              }
            }
          }

          return set;
        }

        QSet<QPersistentModelIndex> persisted (QSet<QModelIndex> set)
        {
          QSet<QPersistentModelIndex> ret;
          for (QModelIndex index : set)
          {
            ret.insert (index);
          }
          return ret;
        }
      }

      void treeview_with_delete::keyPressEvent (QKeyEvent* event)
      {
        if (!(event == QKeySequence::Delete))
        {
          QTreeView::keyPressEvent (event);
          return;
        }

        for ( QPersistentModelIndex index
            : persisted ( indices_with_parent_in_set_removed
                            (list_to_set (selectionModel()->selectedRows()))
                        )
            )
        {
          model()->removeRow (index.row(), index.parent());
        }
      }
    }
  }
}
