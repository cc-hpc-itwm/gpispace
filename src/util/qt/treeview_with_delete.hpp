// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <QTreeView>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class treeview_with_delete : public QTreeView
      {
        Q_OBJECT

      public:
        treeview_with_delete (QWidget* parent = nullptr);

      protected:
        virtual void keyPressEvent (QKeyEvent*) override;
      };
    }
  }
}
