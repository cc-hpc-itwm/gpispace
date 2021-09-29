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

#include <util/qt/mvc/filter_ignoring_branch_nodes_proxy.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        filter_ignoring_branch_nodes_proxy::filter_ignoring_branch_nodes_proxy
          (QAbstractItemModel* model, QObject* parent)
            : sort_filter_proxy (parent)
        {
          setSourceModel (model);
        }

        bool filter_ignoring_branch_nodes_proxy::filterAcceptsRow
          (int source_row, QModelIndex const& parent) const
        {
          if (sort_filter_proxy::filterAcceptsRow (source_row, parent))
          {
            return true;
          }

          const QModelIndex idx
            (sourceModel()->index (source_row, filterKeyColumn(), parent));

          // p is visible if \exists child^n (p) \in visible.

          for (int row (0); row < sourceModel()->rowCount (idx); ++row)
          {
            if (filterAcceptsRow (row, idx))
            {
              return true;
            }
          }

          // ! \todo Is l visible, if \exists parent^n (l) \in visible?

          return false;
        }
      }
    }
  }
}
