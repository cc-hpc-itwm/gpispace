// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/StructureView.hpp>

#include <pnete/weaver/tv.hpp>

#include <QHeaderView>
#include <QStandardItemModel>

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

      void StructureView::append (const data::handle::function& function)
      {
        _functions.push_back (function);

        weaver::treeview::function (_root, function);
      }

      void StructureView::doubleClicked (const QModelIndex& index)
      {
        QStandardItem *item = _model->itemFromIndex (index);
        while (item->parent())
        {
          item = item->parent();
        }

        const int row (item->row());

        weaver::treeview::function (_root, _functions.at (row));

        _model->setItem (row, _model->takeItem (_model->rowCount() - 1));
        _model->setRowCount (_model->rowCount() - 1);
      }

      void StructureView::clear()
      {
        setModel (_model = new QStandardItemModel (this));
        _root = _model->invisibleRootItem();
        _functions.clear();
      }
    }
  }
}
