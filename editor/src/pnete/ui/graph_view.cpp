// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph_view.hpp>

#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/size.hpp>

#include <util/phi.hpp>
#include <util/qt/boost_connect.hpp>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

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
        setAcceptDrops (true);

        setFocusPolicy (Qt::WheelFocus);

        QAction* zoom_in (new QAction (tr ("zoom_in"), this));
        fhg::util::qt::boost_connect<void()>
          ( zoom_in
          , SIGNAL (triggered())
          , this
          , boost::lambda::bind ( &graph_view::zoom
                                , this
                                , boost::lambda::var (_currentScale)
                                + size::zoom::per_tick()
                                )
          );
        zoom_in->setShortcuts (QKeySequence::ZoomIn);
        addAction (zoom_in);

        QAction* zoom_out (new QAction (tr ("zoom_out"), this));
        fhg::util::qt::boost_connect<void()>
          ( zoom_out
          , SIGNAL (triggered())
          , this
          , boost::lambda::bind ( &graph_view::zoom
                                , this
                                , boost::lambda::var (_currentScale)
                                - size::zoom::per_tick()
                                )
          );
        zoom_out->setShortcuts (QKeySequence::ZoomOut);
        addAction (zoom_out);

        QAction* zoom_default (new QAction (tr ("zoom_default"), this));
        fhg::util::qt::boost_connect<void()>
          ( zoom_default
          , SIGNAL (triggered())
          , this
          , boost::lambda::bind ( &graph_view::zoom
                                , this
                                , size::zoom::default_value()
                                )
          );
        zoom_default->setShortcut (QKeySequence ("Ctrl+*"));
        addAction (zoom_default);

        QAction* sep (new QAction (this));
        sep->setSeparator (true);
        addAction (sep);

        addActions (scene->actions());
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

      void graph_view::wheelEvent (QWheelEvent* event)
      {
        if ( event->modifiers() & Qt::ControlModifier
          && event->orientation() == Qt::Vertical
           )
        {
          zoom ( _currentScale
               + size::zoom::per_tick() * (event->delta() > 0 ? 1 : -1)
               );
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

        emit zoomed (_currentScale);
      }
    }
  }
}
