// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_STRUCTURE_VIEW_HPP
#define _FHG_PNETE_UI_STRUCTURE_VIEW_HPP 1

#include <pnete/data/internal.fwd.hpp>

#include <QTreeView>
#include <QVector>

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

        typedef QVector<data::internal_type*> datas_type;

        datas_type _datas;

      public:
        StructureView (QWidget* parent = 0);

        void append (data::internal_type* data);
        void clear();

      protected slots:
        void doubleClicked (const QModelIndex& index);
      };
    }
  }
}

#endif
