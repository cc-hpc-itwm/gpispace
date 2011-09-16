#ifndef UI_STRUCTURE_VIEW_HPP
#define UI_STRUCTURE_VIEW_HPP 1

#include <QTreeView>

#include <string>
#include <list>

#include <pnete/traverse/weaver.hpp>
#include <pnete/data/internal.hpp>

class QStandardItem;
class QStandardItemModel;
class QWidget;
class QString;

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
      private:
        QStandardItemModel* _model;
        QStandardItem* _root;

        typedef std::list<data::internal::ptr> datas_type;

        datas_type _datas;

      public:
        StructureView (QWidget* parent = 0);

        void append (data::internal::ptr data);
        void clear();
      };
    }
  }
}

#endif
