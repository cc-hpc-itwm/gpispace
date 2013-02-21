// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph_view.hpp>

#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/size.hpp>

#include <util/phi.hpp>
#include <util/qt/boost_connect.hpp>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>
#include <QWheelEvent>
#include <QWidgetAction>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace
      {
        class zoom_widget_action : public QWidgetAction
        {
        public:
          zoom_widget_action (graph_view* graph)
            : QWidgetAction (graph)
            , _graph (graph)
          { }

        protected:
          virtual QWidget* createWidget (QWidget* parent)
          {
            QWidget* base (new QWidget (parent));

            QSlider* bar (new QSlider (Qt::Horizontal, parent));
            bar->setMaximumWidth (size::zoom::slider::max_length());
            bar->setRange (size::zoom::min_value(), size::zoom::max_value());
            bar->setValue (size::zoom::default_value());

            QSpinBox* box (new QSpinBox (parent));
            box->setSuffix ("%");
            box->setRange (size::zoom::min_value(), size::zoom::max_value());
            box->setValue (size::zoom::default_value());

            connect (box, SIGNAL (valueChanged (int)), bar, SLOT (setValue (int)));
            connect (bar, SIGNAL (valueChanged (int)), box, SLOT (setValue (int)));
            connect (bar, SIGNAL (valueChanged (int)), _graph, SLOT (zoom (int)));
            connect (_graph, SIGNAL (zoomed (int)), bar, SLOT (setValue (int)));

            QHBoxLayout* layout (new QHBoxLayout (base));
            layout->addWidget (bar);
            layout->addWidget (box);

            return base;
          }

        private:
          graph_view* _graph;
        };
      }

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

        addAction (new zoom_widget_action (this));

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
