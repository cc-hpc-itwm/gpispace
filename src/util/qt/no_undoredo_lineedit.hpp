// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <QLineEdit>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class no_undoredo_lineedit : public QLineEdit
      {
        Q_OBJECT

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
