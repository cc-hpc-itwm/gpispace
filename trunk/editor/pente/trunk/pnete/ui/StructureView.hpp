#ifndef UI_STRUCTURE_VIEW_HPP
#define UI_STRUCTURE_VIEW_HPP 1

#include <QTreeView>

#include <string>

#include <pnete/traverse/weaver.hpp>

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
        StructureView (QWidget* parent = 0);

        void from_file (const QString & input);

        void from ( const XMLTYPE(function_type) & fun
                  , const XMLPARSE(state::key_values_t) & context
                  = XMLPARSE(state::key_values_t) ()
                  );

      private:
        QStandardItemModel* _model;
        QStandardItem* _root;
      };
    }
  }
}

#endif
