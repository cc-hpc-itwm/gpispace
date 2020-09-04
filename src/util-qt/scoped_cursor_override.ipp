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

#include <QtGui/QCursor>
#include <QtWidgets/QApplication>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      inline scoped_cursor_override::scoped_cursor_override
        (QCursor const& cursor)
      {
        QApplication::setOverrideCursor (cursor);
      }
      inline scoped_cursor_override::~scoped_cursor_override()
      {
        QApplication::restoreOverrideCursor();
      }
    }
  }
}
