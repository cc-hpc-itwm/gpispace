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
        no_undoredo_lineedit (QWidget* parent = nullptr);
        no_undoredo_lineedit (const QString& content, QWidget* parent = nullptr);

      protected:
        virtual void contextMenuEvent (QContextMenuEvent*) override;
        virtual void keyPressEvent (QKeyEvent*) override;
        virtual bool event (QEvent*) override;
      };
    }
  }
}

#endif
