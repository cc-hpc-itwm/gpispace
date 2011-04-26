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

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      GraphView::GraphView(QGraphicsScene* scene, QWidget* parent)
      : QGraphicsView(scene, parent)
      {
        setDragMode(QGraphicsView::ScrollHandDrag);
        setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
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
    }
  }
}
