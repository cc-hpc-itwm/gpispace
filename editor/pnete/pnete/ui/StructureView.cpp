// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/StructureView.hpp>

#include <pnete/weaver/weaver.hpp>
#include <pnete/weaver/tv.hpp>

#include <QStandardItem>
#include <QStandardItemModel>
#include <QWidget>
#include <QHeaderView>
#include <QString>

#include <boost/format.hpp>

#include <stack>

#include <stdexcept>

#include <pnete/data/internal.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      StructureView::StructureView (QWidget* parent)
      : QTreeView (parent)
      , _model (new QStandardItemModel (this))
      , _root (_model->invisibleRootItem())
      {
        setModel (_model);

        //! \todo As soon as we can actually edit stuff in here, remove.
        setDragDropMode (QAbstractItemView::NoDragDrop);
        setEditTriggers (QAbstractItemView::NoEditTriggers);

        header()->hide();
      }

      void StructureView::append (data::internal_type* data)
      {
        _datas.push_back (data);

        weaver::tv w (_root);
        weaver::from::function_context
          ( &w
          , WNAME(function_context_type) (data->function(), data->context())
          );
      }

      void StructureView::clear()
      {
        setModel (_model = new QStandardItemModel (this));
        _root = _model->invisibleRootItem();
        _datas.clear();
      }
    }
  }
}
