// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_STRUCTURE_VIEW_HPP
#define _FHG_PNETE_UI_STRUCTURE_VIEW_HPP 1

#include <QTreeView>

#include <string>
#include <list>

#include <pnete/weaver/weaver.hpp>

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
    namespace data
    {
      class internal_type;
    }

    namespace ui
    {
      class StructureView : public QTreeView
      {
      private:
        QStandardItemModel* _model;
        QStandardItem* _root;

        typedef std::list<data::internal_type*> datas_type;

        datas_type _datas;

      public:
        StructureView (QWidget* parent = 0);

        void append (data::internal_type* data);
        void clear();
      };
    }
  }
}

#endif
