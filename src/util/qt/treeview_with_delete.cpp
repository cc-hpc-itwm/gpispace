// Copyright (C) 2013-2014,2020,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/qt/compat.hpp>
#include <gspc/util/qt/treeview_with_delete.hpp>

#include <QKeyEvent>
#include <QList>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QSet>
#include <QVector>



    namespace gspc::util::qt
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
          for (auto index : set)
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
