// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_STRUCTURE_VIEW_HPP
#define _FHG_PNETE_UI_STRUCTURE_VIEW_HPP 1

#include <pnete/data/handle/function.hpp>

#include <QList>
#include <QTreeView>

class QStandardItem;
class QStandardItemModel;
class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class StructureView : public QTreeView
      {
        Q_OBJECT;

      private:
        QStandardItemModel* _model;
        QStandardItem* _root;

        QList<data::handle::function> _functions;

      public:
        StructureView (QWidget* parent = 0);

        void append (const data::handle::function&);
        void clear();

      protected slots:
        void doubleClicked (const QModelIndex& index);
      };
    }
  }
}

#endif
