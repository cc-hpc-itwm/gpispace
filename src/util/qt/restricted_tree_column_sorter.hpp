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

#pragma once

#include <QObject>
#include <QSet>

class QTreeView;

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class restricted_tree_column_sorter : public QObject
      {
        Q_OBJECT

      public:
        restricted_tree_column_sorter
          (QTreeView*, QSet<int> allowed_columns, QObject* parent = nullptr);

      private:
        enum state
        {
          none,
          asc,
          desc
        };

        void next (int column);

        QTreeView* _tree;
        QSet<int> _allowed_columns;
        state _last_state;
        int _last_column;
      };
    }
  }
}
