// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
