// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/StructureView.hpp>

#include <pnete/weaver/tv.hpp>
#include <pnete/weaver/weaver.hpp>

#include <QStandardItemModel>
#include <QHeaderView>

#include <pnete/data/internal.hpp>

#include <QDebug>

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

        connect ( this, SIGNAL (doubleClicked(QModelIndex))
                , this, SLOT (doubleClicked(QModelIndex))
                );
      }

      void StructureView::append (data::internal_type* data)
      {
        _datas.push_back (data);

        weaver::treeview::function (_root, data);
      }

      void StructureView::doubleClicked (const QModelIndex& index)
      {
        QStandardItem *item = _model->itemFromIndex (index);
        while (item->parent())
        {
          item = item->parent();
        }

        const int row (item->row());

        weaver::treeview::function (_root, _datas.at (row));

        _model->setItem (row, _model->takeItem (_model->rowCount() - 1));
        _model->setRowCount (_model->rowCount() - 1);
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
