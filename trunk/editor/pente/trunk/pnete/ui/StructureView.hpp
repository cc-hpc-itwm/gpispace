#ifndef UI_STRUCTURE_VIEW_HPP
#define UI_STRUCTURE_VIEW_HPP 1

#include <QTreeView>

class QStandardItem;
class QStandardItemModel;
class QWidget;

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct function_type;
    }
  }
}

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class StructureView : public QTreeView
      {
      public:
        StructureView (QWidget* parent = 0);

        void from (const ::xml::parse::type::function_type * fun);

      private:
        QStandardItemModel* _model;
        QStandardItem* _root;
      };
    }
  }
}

#endif
