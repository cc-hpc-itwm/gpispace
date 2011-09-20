
#include "GraphConnection.hpp"
#include "GraphConnectableItem.hpp"
#include "GraphStyle.hpp"
#include "GraphPort.hpp"
#include "GraphScene.hpp"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

class QGraphicsScene;

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Connection::Connection()
      : graph_item()
      , _start (NULL)
      , _end (NULL)
      , _highlighted (false)
      {
        setZValue (-1);                                                          // hardcoded constant
      }

      void Connection::setStart(ConnectableItem* start)
      {
        if(start)
        {
          start->connectMe(this);
        }
        else if(_start)
        {
          _start->disconnectMe();
        }
        _start = start;
        recalcMidpoints();
      }
      void Connection::setEnd(ConnectableItem* end)
      {
        if(end)
        {
          end->connectMe(this);
        }
        else if(_end)
        {
          _end->disconnectMe();
        }
        _end = end;
        recalcMidpoints();
      }

      void Connection::removeMe(ConnectableItem* item)
      {
        if(_end == item)
        {
          _end->disconnectMe();
          _end = NULL;
        }
        else if(_start == item)
        {
          _start->disconnectMe();
          _start = NULL;
        }
        recalcMidpoints();
      }

      const ConnectableItem* Connection::start() const
      {
        return _start;
      }
      const ConnectableItem* Connection::end() const
      {
        return _end;
      }

      QPointF addInOrientationDirection(const QPointF& point, ConnectableItem::eOrientation orientation, qreal value)
      {
        static const QPointF horizontal( value, 0.0 );
        static const QPointF vertical( 0.0, value );

        QPointF ret = point;
        switch(orientation)
        {
          case ConnectableItem::NORTH:
            ret -= vertical;
            break;
          case ConnectableItem::SOUTH:
            ret += vertical;
            break;
          case ConnectableItem::EAST:
            ret += horizontal;
            break;
          case ConnectableItem::WEST:
            ret -= horizontal;
            break;
          case ConnectableItem::ANYORIENTATION:
          default:
            break;
        }
        return ret;
      }

      const QPointF Connection::startPosition() const
      {
        QPointF position;
        if (scene())
        {
          if (start())
          {
            position = start()->scenePos();
            const Port* port = qgraphicsitem_cast<const Port*>(start());
            if (port)
            {
              qreal lengthHalf = port->length() / 2.0;                          // hardcoded constant
              position = addInOrientationDirection(position, port->orientation(), lengthHalf);
            }
          }
          else
          {
            position = scene()->mousePosition();
          }
          position -= scenePos();
        }
        return position;
      }
      const QPointF Connection::endPosition() const
      {
        QPointF position;
        if (scene())
        {
          if (end())
          {
            position = end()->scenePos();
            const Port* port = qgraphicsitem_cast<const Port*>(end());
            if(port)
            {
              qreal lengthHalf = port->length() / 2.0;                          // hardcoded constant
              position = addInOrientationDirection(position, port->orientation(), lengthHalf);
            }
          }
          else
          {
            position = scene()->mousePosition();
          }
          position -= scenePos();
        }
        return position;
      }

      QPainterPath Connection::shape() const
      {
        return Style::connectionShape(this);
      }

      QRectF Connection::boundingRect() const
      {
        return Style::connectionBoundingRect(this);
      }

      void Connection::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
        recalcMidpoints();
        Style::connectionPaint(painter, this);
      }

      const QList<QPointF>& Connection::midpoints() const
      {
        return _midpoints;
      }

      const bool& Connection::highlighted() const
      {
        return _highlighted;
      }

      void Connection::recalcMidpoints()
      {
        _midpoints.clear();
        if(!start() && !end())
        {
          return;
        }

        QPointF startPoint = startPosition();
        QPointF endPoint = endPosition();

#ifdef STEFAN
        QPointF midPoint = start() ? ( end() ? QLineF(startPoint, endPoint).pointAt(0.5) : endPoint ) : startPoint;

        const qreal padding = Style::raster() * 3.0;                            // hardcoded constant

        if(start())
        {
          QRectF transitionBounding = start()->parentItem()->sceneBoundingRect();
          QPointF transitionBottomRight = transitionBounding.bottomRight();
          QPointF transitionTopLeft = transitionBounding.topLeft();

          QPointF posdiff = startPoint + midPoint;
          QPointF portdiff = transitionTopLeft + transitionBottomRight;

          transitionBounding.adjust(-padding, -padding, padding, padding);

          _midpoints.push_back(midPoint);
          if(posdiff.x() < portdiff.x())
          {
            if(posdiff.y() < portdiff.y())
            {
              _midpoints.push_back(QPointF(midPoint.x(), transitionBounding.topLeft().y()));
              _midpoints.push_back(QPointF(startPoint.x(), transitionBounding.topLeft().y()));
             // pos = UPPERLEFT;
            }
            else
            {
              _midpoints.push_back(QPointF(transitionBounding.topLeft().x(), midPoint.y()));
              _midpoints.push_back(QPointF(transitionBounding.topLeft().x(), transitionBounding.topLeft().y()));
              _midpoints.push_back(QPointF(startPoint.x(), transitionBounding.topLeft().y()));
            //  pos = LOWERLEFT;
            }
          }
          else
          {
            if(posdiff.y() < portdiff.y())
            {
              _midpoints.push_back(QPointF(transitionBounding.bottomRight().x(), midPoint.y()));
              _midpoints.push_back(QPointF(transitionBounding.bottomRight().x(), transitionBounding.topLeft().y()));
              _midpoints.push_back(QPointF(startPoint.x(), transitionBounding.topLeft().y()));
             // pos = UPPERRIGHT;
            }
            else
            {
              _midpoints.push_back(QPointF(midPoint.x(), transitionBounding.bottomRight().y()));
              _midpoints.push_back(QPointF(startPoint.x(), transitionBounding.bottomRight().y()));
             // pos = LOWERRIGHT;
            }
          }
          _midpoints.push_back(startPoint);
        }

        if(end())
        {
         // QRectF endTransitionBounding = end()->parentItem()->sceneBoundingRect().adjust(-padding, -padding, padding, padding);
        }
#endif

        ConnectableItem::eOrientation startOrientation = start() ? start()->orientation() : ConnectableItem::EAST;
        ConnectableItem::eOrientation endOrientation = end() ? end()->orientation() : ConnectableItem::WEST;

        QLineF startLine(startPoint, addInOrientationDirection(startPoint, startOrientation, 1.0));   // hardcoded constant
        QLineF endLine(endPoint, addInOrientationDirection(endPoint, endOrientation, 1.0));           // hardcoded constant

        //! \todo more than these simple cases.
        QPointF intersection;
        if(startLine.intersect(endLine, &intersection) == QLineF::NoIntersection)
        {
          QPointF mid = QLineF(startPoint, endPoint).pointAt(0.5);
          if(startOrientation == ConnectableItem::NORTH || startOrientation == ConnectableItem::SOUTH)
          {
            if(startPoint.x() != endPoint.x())
            {
              _midpoints.push_back(QPointF(startPoint.x(), mid.y()));
              _midpoints.push_back(QPointF(endPoint.x(), mid.y()));
            }
          }
          else
          {
            if(startPoint.y() != endPoint.y())
            {
              _midpoints.push_back(QPointF(mid.x(), startPoint.y()));
              _midpoints.push_back(QPointF(mid.x(), endPoint.y()));
            }
          }
        }
        else
        {
          _midpoints.push_back(intersection);
        }
      }
    }
  }
}
