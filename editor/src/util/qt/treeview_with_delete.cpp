// bernd.loerwald@itwm.fraunhofer.de

#include <util/qt/treeview_with_delete.hpp>

#include <QKeyEvent>
#include <QList>
#include <QModelIndex>
#include <QPersistentModelIndex>
#include <QSet>
#include <QVector>

#include <boost/foreach.hpp>

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
          BOOST_FOREACH (QModelIndex index, set)
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

        BOOST_FOREACH ( QPersistentModelIndex index
                      , persisted ( indices_with_parent_in_set_removed
                                    (selectionModel()->selectedRows().toSet())
                                  )
                      )
        {
          model()->removeRow (index.row(), index.parent());
        }
      }
    }
  }
}
