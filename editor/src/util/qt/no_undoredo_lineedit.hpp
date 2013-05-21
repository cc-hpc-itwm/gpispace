// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_NO_UNDOREDO_LINEEDIT_HPP
#define FHG_UTIL_QT_NO_UNDOREDO_LINEEDIT_HPP

#include <QLineEdit>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class no_undoredo_lineedit : public QLineEdit
      {
        Q_OBJECT;

      public:
        no_undoredo_lineedit (QWidget* parent = NULL);
        no_undoredo_lineedit (const QString& content, QWidget* parent = NULL);

      protected:
        virtual void contextMenuEvent (QContextMenuEvent*);
        virtual void keyPressEvent (QKeyEvent*);
        virtual bool event (QEvent*);
      };
    }
  }
}

#endif
