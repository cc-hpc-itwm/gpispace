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
