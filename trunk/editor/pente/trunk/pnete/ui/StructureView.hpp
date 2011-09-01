#ifndef UI_STRUCTURE_VIEW_HPP
#define UI_STRUCTURE_VIEW_HPP 1

#include <QTreeView>

#include <string>

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
      public:
        StructureView (const QString & load, QWidget* parent = 0);

        void fromFile (const std::string & input);
        void from (const ::xml::parse::type::function_type & fun);

      private:
        QStandardItemModel* _model;
        QStandardItem* _root;
      };
    }
  }
}

#endif
