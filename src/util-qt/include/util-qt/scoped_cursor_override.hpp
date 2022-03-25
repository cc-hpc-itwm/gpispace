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

#pragma once

#include <util-qt/forward_decl.hpp>

FHG_UTIL_QT_FORWARD_DECL (class QCursor);

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      //! RAII wrapper for QApplication's override cursor. Overrides
      //! the currently used cursor. Correctly handles nested
      //! overriding.
      struct scoped_cursor_override
      {
        scoped_cursor_override (QCursor const&);
        ~scoped_cursor_override();

        scoped_cursor_override() = delete;
        scoped_cursor_override (scoped_cursor_override const&) = delete;
        scoped_cursor_override (scoped_cursor_override&&) = delete;
        scoped_cursor_override& operator= (scoped_cursor_override const&) = delete;
        scoped_cursor_override& operator= (scoped_cursor_override&&) = delete;
      };
    }
  }
}

#include <util-qt/scoped_cursor_override.ipp>
