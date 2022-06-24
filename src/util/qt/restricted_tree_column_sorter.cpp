// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util/qt/restricted_tree_column_sorter.hpp>

#include <QHeaderView>
#include <QTreeView>

#include <functional>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      restricted_tree_column_sorter::restricted_tree_column_sorter
        (QTreeView* tree, QSet<int> allowed_columns, QObject* parent)
          : QObject (parent)
          , _tree (tree)
          , _allowed_columns (allowed_columns)
      {
        _tree->header()->setSectionsClickable (true);
        connect
          ( _tree->header(), &QHeaderView::sectionClicked
          , std::bind (&restricted_tree_column_sorter::next, this, std::placeholders::_1)
          );
        next (*_allowed_columns.begin());
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
