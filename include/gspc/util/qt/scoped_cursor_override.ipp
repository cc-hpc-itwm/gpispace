#include <QtGui/QCursor>
#include <QtWidgets/QApplication>

namespace gspc
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
