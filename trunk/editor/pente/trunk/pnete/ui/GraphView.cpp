// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/GraphView.hpp>

#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QWheelEvent>

#include <pnete/ui/TransitionLibraryModel.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/size.hpp>

#include <pnete/ui/graph/style/raster.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      GraphView::GraphView (graph::scene::type* scene, QWidget* parent)
      : QGraphicsView (scene, parent)
      , _currentScale (size::zoom::default_value())
      {
        setDragMode (QGraphicsView::ScrollHandDrag);
        setRenderHints ( QPainter::Antialiasing
                       | QPainter::TextAntialiasing
                       | QPainter::SmoothPixmapTransform
                       );
      }

      QSize GraphView::sizeHint() const
      {
        return QSize (window()->width() * 0.8, window()->height());
      }

      graph::scene::type* GraphView::scene() const
      {
        return qobject_cast<graph::scene::type*> (QGraphicsView::scene());
      }

      void GraphView::focusInEvent (QFocusEvent* event)
      {
        emit focus_gained (this);
        QGraphicsView::focusInEvent (event);
      }

      void GraphView::dragEnterEvent (QDragEnterEvent* event)
      {
        //! \todo Paint a ghost transition.
        const QMimeData* mimeData (event->mimeData());
        if (mimeData->hasFormat (TransitionLibraryModel::mimeType))
        {
          event->acceptProposedAction();
          return;
        }
        else
        {
          QGraphicsView::dragEnterEvent (event);
        }
      }

      void GraphView::dragMoveEvent (QDragMoveEvent* event)
      {
        //! \todo Paint a ghost transition.
        const QMimeData* mimeData (event->mimeData());
        if (mimeData->hasFormat (TransitionLibraryModel::mimeType))
        {
          event->acceptProposedAction();
        }
        else
        {
          QGraphicsView::dragMoveEvent (event);
        }
      }

      void GraphView::dropEvent (QDropEvent* event)
      {
//         const QMimeData* mimeData (event->mimeData());
//         if (mimeData->hasFormat (TransitionLibraryModel::mimeType))
//         {
//           //! \todo get path.
//           const QString filename/* (magic)*/;
//           // mimeData->data(TransitionLibraryModel::mimeType) === filepath as ..)

//           graph::transition::item* transition
//             (new graph::transition::item (filename));

//           scene()->addItem (transition);

//           transition->setPos
//             ( graph::style::raster::snap
//               ( mapToScene (event->pos())
//               - transition->boundingRect().bottomRight() / 2.0
//               )
//             );

//           event->acceptProposedAction();
//         }
//         else
//         {
//           QGraphicsView::dropEvent (event);
//         }
      }

      void GraphView::wheelEvent (QWheelEvent* event)
      {
        if ( event->modifiers() & Qt::ControlModifier
          && event->orientation() == Qt::Vertical
           )
        {
          setFocus();
          if (event->delta() > 0)
          {
            zoom_in();
          }
          else
          {
            zoom_out();
          }
        }
        else
        {
          QGraphicsView::wheelEvent (event);
        }
      }

      void GraphView::zoom (int to)
      {
        const int target (qBound ( size::zoom::min_value()
                                 , to
                                 , size::zoom::max_value()
                                 )
                         );
        const qreal factor (target / static_cast<qreal> (_currentScale));
        scale (factor, factor);
        _currentScale = target;

        emit_current_zoom_level();
      }

      void GraphView::zoom_in()
      {
        zoom (_currentScale + size::zoom::per_tick());
      }
      void GraphView::zoom_out()
      {
        zoom (_currentScale - size::zoom::per_tick());
      }
      void GraphView::reset_zoom()
      {
        zoom (size::zoom::default_value());
      }

      void GraphView::emit_current_zoom_level()
      {
        emit zoomed (_currentScale);
      }
    }
  }
}
