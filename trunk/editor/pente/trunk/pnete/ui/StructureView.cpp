#include "StructureView.hpp"

#include <QStandardItem>
#include <QStandardItemModel>
#include <QWidget>
#include <QHeaderView>

#include <xml/parse/types.hpp>
#include <fhg/util/maybe.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace detail
      {
      }

      // ******************************************************************* //

      StructureView::StructureView (QWidget* parent)
        : QTreeView (parent)
        , _model (new QStandardItemModel(this))
        , _root (_model->invisibleRootItem())
      {
        setModel (_model);

        setFrameShape(QFrame::StyledPanel);
        setFrameShadow(QFrame::Sunken);
        setDragDropMode(QAbstractItemView::DragOnly);
        header()->setVisible(false);

        QStandardItem* phi (new QStandardItem("phi"));
        QStandardItem* psi (new QStandardItem("psi"));
        QStandardItem* rho (new QStandardItem("rho"));

        _root->appendRow (phi);
        _root->appendRow (psi);
        psi->appendRow(rho);
      }

      void StructureView::from (const ::xml::parse::type::function_type * fun)
      {
        std::cerr << "FROM " << (fun->name.isJust() ? *fun->name : "<fun without a name>")
                  << std::endl
          ;
      }
    }
  }
}
