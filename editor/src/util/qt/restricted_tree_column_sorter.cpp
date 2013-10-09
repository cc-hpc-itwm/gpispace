// bernd.loerwald@itwm.fraunhofer.de

#include <util/qt/restricted_tree_column_sorter.hpp>

#include <util/qt/boost_connect.hpp>

#include <boost/bind.hpp>

#include <QHeaderView>
#include <QTreeView>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      restricted_tree_column_sorter::restricted_tree_column_sorter
        (QTreeView* tree, const QSet<int> allowed_columns)
          : _tree (tree)
          , _allowed_columns (allowed_columns)
          , _last_state (none)
          , _last_column (-1)
      {
        _tree->header()->setClickable (true);
        util::qt::boost_connect<void (int)>
          ( _tree->header(), SIGNAL (sectionClicked (int))
          , boost::bind (&restricted_tree_column_sorter::next, this, _1)
          );
      }

      void restricted_tree_column_sorter::next (int column)
      {
        if (!_allowed_columns.contains (column))
        {
          //! \note: before sectionClicked is emitted, there is
          //! _always_ a non-supressable sort indicator flip by Qt
          _tree->header()->setSortIndicator
            ( _last_state == none ? -1 : _last_column
            , _last_state == asc ? Qt::AscendingOrder : Qt::DescendingOrder
            );
        }
        else
        {
          const state new_state
            ( _last_state == none || _last_column != column ? asc
            : _last_state == asc ? desc
            : none
            );

          _tree->sortByColumn
            ( new_state == none ? -1 : column
            , new_state == asc ? Qt::AscendingOrder : Qt::DescendingOrder
            );

          _last_column = column;
          _last_state = new_state;
        }
      }
    }
  }
}
