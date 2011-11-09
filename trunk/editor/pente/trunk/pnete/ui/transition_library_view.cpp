// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/transition_library_view.hpp>

#include <QWidget>
#include <QResizeEvent>
#include <QHeaderView>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      transition_library_view::transition_library_view ( int width1
                                                       , int margin
                                                       , QWidget* parent
                                                       )
        : QTreeView (parent)
        , _width1 (width1)
        , _margin (margin)
      {
        setColumnWidth (1, _width1);
        setDragDropMode (QAbstractItemView::DragOnly);
        setSelectionMode (QAbstractItemView::ExtendedSelection);
        setDragEnabled (true);
        header()->hide();
      }

      void transition_library_view::resizeEvent (QResizeEvent* event)
      {
        QTreeView::resizeEvent (event);

        setColumnWidth (0, size().width() - _width1 - _margin);
      }
    }
  }
}
