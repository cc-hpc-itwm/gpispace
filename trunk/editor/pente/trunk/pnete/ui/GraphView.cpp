#include "GraphView.hpp"
#include "graph/Transition.hpp"
#include "graph/Port.hpp"
#include "graph/Style.hpp"

#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPainter>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      static const QString acceptedMimetype = "text/uri-list";
      
      GraphView::GraphView(QWidget* parent)
      : QGraphicsView(parent)
      {
        setDragMode(QGraphicsView::ScrollHandDrag);
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
      }

      void GraphView::dragEnterEvent(QDragEnterEvent *event)
      {
        const QMimeData *mimeData = event->mimeData();
        if( mimeData->hasFormat( acceptedMimetype ) )
        {
          event->acceptProposedAction();
        }
      }
      
      void GraphView::dragMoveEvent(QDragMoveEvent *event)
      {
        const QMimeData *mimeData = event->mimeData();
        if( mimeData->hasFormat( acceptedMimetype ) )
        {
          event->acceptProposedAction();
        }
      }
      
      void GraphView::dropEvent(QDropEvent *event)
      {
        const QMimeData *mimeData = event->mimeData();
        if( mimeData->hasFormat( acceptedMimetype ) )
        {
            //Transition trans; trans.name = mimeData->data( acceptedMimetype ).data();
          graph::Transition* stb = new graph::Transition();
          new graph::Port(stb,graph::Port::IN);
          new graph::Port(stb,graph::Port::IN);
          new graph::Port(stb,graph::Port::OUT);
          new graph::Port(stb,graph::Port::OUT);
          scene()->addItem(stb);
          stb->setPos(graph::Style::snapToRaster(mapToScene(event->pos())));
          event->acceptProposedAction();
        }
      }
    }
  }
}
