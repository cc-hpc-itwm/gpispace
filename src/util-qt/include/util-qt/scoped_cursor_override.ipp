// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
