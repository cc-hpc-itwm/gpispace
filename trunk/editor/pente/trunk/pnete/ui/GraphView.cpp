#include "GraphView.hpp"
#include "TransitionLibraryModel.hpp"
#include "graph/Transition.hpp"
#include "graph/Port.hpp"
#include "graph/Style.hpp"
#include "data/Transition.hpp"
#include "data/Port.hpp"

#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPainter>
#include <QWheelEvent>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      GraphView::GraphView(QGraphicsScene* scene, QWidget* parent)
      : QGraphicsView(scene, parent),
      _currentScale(1.0)
      {
        setDragMode(QGraphicsView::ScrollHandDrag);
        setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
      }

      void GraphView::dragEnterEvent(QDragEnterEvent *event)
      {
        //! \todo Paint a ghost transition.
        const QMimeData *mimeData = event->mimeData();
        if(mimeData->hasFormat(TransitionLibraryModel::mimeType))
        {
          event->acceptProposedAction();
        }
      }
      
      void GraphView::dragMoveEvent(QDragMoveEvent *event)
      {
        //! \todo Paint a ghost transition.
        const QMimeData *mimeData = event->mimeData();
        if(mimeData->hasFormat(TransitionLibraryModel::mimeType))
        {
          event->acceptProposedAction();
        }
      }
      
      //! \todo Move somewhere else.
      graph::Transition* createTransitionFromMimeData(const QByteArray& data)
      {
        QByteArray byteArray(data);
        data::Transition transitionData;
        QDataStream stream(byteArray);
        stream >> transitionData;
        
        graph::Transition* transition = new graph::Transition(transitionData.name());
        foreach(data::Port port, transitionData.inPorts())
        {
          new graph::Port(transition, graph::Port::IN, port.name(), port.type());
        }
        foreach(data::Port port, transitionData.outPorts())
        {
          new graph::Port(transition, graph::Port::OUT, port.name(), port.type());
        }
        transition->repositionChildrenAndResize();
        
        return transition;
      }
      
      void GraphView::dropEvent(QDropEvent *event)
      {
        const QMimeData *mimeData = event->mimeData();
        if(mimeData->hasFormat(TransitionLibraryModel::mimeType))
        {
          graph::Transition* transition = createTransitionFromMimeData(mimeData->data(TransitionLibraryModel::mimeType));
          scene()->addItem(transition);
          transition->setPos(graph::Style::snapToRaster(mapToScene(event->pos())));
          event->acceptProposedAction();
        }
      }
      
      void GraphView::wheelEvent(QWheelEvent* event)
      {
        if(event->modifiers() & Qt::ControlModifier && event->orientation() == Qt::Vertical)
        {
          //! \note magic number taken from QWheelEvent::delta() documentation.
          //! \note for the love of god, don't remove the +0.005.
          int current = static_cast<int>((_currentScale + 0.005) * 100.0);
          int plus = event->delta() > 0 ? 5 : -5;
          //! \todo max and min zoom level from somewhere, not hardcoded?
          int to = std::max(10, std::min(300, current + plus));
          zoom(to);
        }
        else
        {
          QGraphicsView::wheelEvent(event);
        }
      }
      
      void GraphView::zoom(int to)
      {
        qreal target = (to / 100.0);
        qreal factor = target / _currentScale;
        scale(factor, factor);
        _currentScale = target;
        
        emit zoomed(to);
      }
    }
  }
}
