// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph_view.hpp>

#include <pnete/data/manager.hpp>
#include <pnete/ui/TransitionLibraryItem.hpp>
#include <pnete/ui/TransitionLibraryModel.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/size.hpp>

#include <util/phi.hpp>

#include <xml/parse/type/net.hpp>

#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QWheelEvent>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      graph_view::graph_view (graph::scene_type* scene, QWidget* parent)
      : QGraphicsView (scene, parent)
      , _currentScale (size::zoom::default_value())
      {
        setDragMode (QGraphicsView::ScrollHandDrag);
        setRenderHints ( QPainter::Antialiasing
                       | QPainter::TextAntialiasing
                       | QPainter::SmoothPixmapTransform
                       );
      }

      QSize graph_view::sizeHint() const
      {
        return QSize ( util::phi::ratio::smaller (window()->width())
                     , window()->height()
                     );
      }

      graph::scene_type* graph_view::scene() const
      {
        return qobject_cast<graph::scene_type*> (QGraphicsView::scene());
      }

      void graph_view::focusInEvent (QFocusEvent* event)
      {
        emit focus_gained (this);
        QGraphicsView::focusInEvent (event);
      }

      void graph_view::dragEnterEvent (QDragEnterEvent* event)
      {
        QGraphicsView::dragEnterEvent (event);

        //! \todo Paint a ghost transition.
        const QMimeData* mimeData (event->mimeData());

        if (mimeData->hasFormat (TransitionLibraryModel::mimeType))
        {
          event->acceptProposedAction();
        }
      }

      void graph_view::dragMoveEvent (QDragMoveEvent* event)
      {
        QGraphicsView::dragMoveEvent (event);

        //! \todo Paint a ghost transition.
        const QMimeData* mimeData (event->mimeData());

        if (mimeData->hasFormat (TransitionLibraryModel::mimeType))
        {
          event->acceptProposedAction();
        }
      }

      void graph_view::dropEvent (QDropEvent* event)
      {
        QGraphicsView::dropEvent (event);

        const QMimeData* mimeData (event->mimeData());

        if (mimeData->hasFormat (TransitionLibraryModel::mimeType))
        {
          QByteArray byteArray
            (mimeData->data (TransitionLibraryModel::mimeType));
          QDataStream stream (&byteArray, QIODevice::ReadOnly);

          QSet<QString> paths;

          stream >> paths;

          foreach (const QString& path, paths)
            {
              data::internal_type* data
                (data::manager::instance().load (path));

              scene()->net().add_transition
                ( scene()
                , data->function().get().clone
                  ( ::xml::parse::type::function_type::make_parent
                    (scene()->net().id().id())
                  , scene()->net().id().id_mapper()
                  )
                );
            }

          event->acceptProposedAction();
        }
      }

      void graph_view::wheelEvent (QWheelEvent* event)
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

      void graph_view::zoom (int to)
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

      void graph_view::zoom_in()
      {
        zoom (_currentScale + size::zoom::per_tick());
      }
      void graph_view::zoom_out()
      {
        zoom (_currentScale - size::zoom::per_tick());
      }
      void graph_view::reset_zoom()
      {
        zoom (size::zoom::default_value());
      }

      void graph_view::emit_current_zoom_level()
      {
        emit zoomed (_currentScale);
      }
    }
  }
}
