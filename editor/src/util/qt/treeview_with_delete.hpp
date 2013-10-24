// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_TREEVIEW_WITH_DELETE_HPP
#define FHG_UTIL_QT_TREEVIEW_WITH_DELETE_HPP

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
        treeview_with_delete (QWidget* parent = NULL);

      protected:
        virtual void keyPressEvent (QKeyEvent*);
      };
    }
  }
}

#endif
