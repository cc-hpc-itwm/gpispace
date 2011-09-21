// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/style.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/scene.hpp>

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

class QGraphicsScene;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        connection::connection()
          : item()
          , _start (NULL)
          , _end (NULL)
          , _highlighted (false)
        {
          setZValue (-1);                                                          // hardcoded constant
        }

        void connection::setStart(connectable_item* start)
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
        void connection::setEnd(connectable_item* end)
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

        void connection::removeMe(connectable_item* item)
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

        const connectable_item* connection::start() const
        {
          return _start;
        }
        const connectable_item* connection::end() const
        {
          return _end;
        }

        QPointF addInOrientationDirection(const QPointF& point, connectable_item::eOrientation orientation, qreal value)
        {
          static const QPointF horizontal( value, 0.0 );
          static const QPointF vertical( 0.0, value );

          QPointF ret = point;
          switch(orientation)
          {
          case connectable_item::NORTH:
            ret -= vertical;
            break;
          case connectable_item::SOUTH:
            ret += vertical;
            break;
          case connectable_item::EAST:
            ret += horizontal;
            break;
          case connectable_item::WEST:
            ret -= horizontal;
            break;
          case connectable_item::ANYORIENTATION:
          default:
            break;
          }
          return ret;
        }

        const QPointF connection::startPosition() const
        {
          QPointF position;
          if (scene())
          {
            if (start())
            {
              position = start()->scenePos();
              const port* item = qgraphicsitem_cast<const port*>(start());
              if (item)
              {
                qreal lengthHalf = item->length() / 2.0;                          // hardcoded constant
                position = addInOrientationDirection(position, item->orientation(), lengthHalf);
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
        const QPointF connection::endPosition() const
        {
          QPointF position;
          if (scene())
          {
            if (end())
            {
              position = end()->scenePos();
              const port* item = qgraphicsitem_cast<const port*>(end());
              if (item)
              {
                qreal lengthHalf = item->length() / 2.0;                          // hardcoded constant
                position = addInOrientationDirection(position, item->orientation(), lengthHalf);
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

        QPainterPath connection::shape() const
        {
          return style::connectionShape(this);
        }

        QRectF connection::boundingRect() const
        {
          return style::connectionBoundingRect(this);
        }

        void connection::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
        {
          recalcMidpoints();
          style::connectionPaint(painter, this);
        }

        const QList<QPointF>& connection::midpoints() const
        {
          return _midpoints;
        }

        const bool& connection::highlighted() const
        {
          return _highlighted;
        }

        void connection::recalcMidpoints()
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

          const qreal padding = style::raster() * 3.0;                            // hardcoded constant

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

          connectable_item::eOrientation startOrientation = start() ? start()->orientation() : connectable_item::EAST;
          connectable_item::eOrientation endOrientation = end() ? end()->orientation() : connectable_item::WEST;

          QLineF startLine(startPoint, addInOrientationDirection(startPoint, startOrientation, 1.0));   // hardcoded constant
          QLineF endLine(endPoint, addInOrientationDirection(endPoint, endOrientation, 1.0));           // hardcoded constant

          //! \todo more than these simple cases.
          QPointF intersection;
          if(startLine.intersect(endLine, &intersection) == QLineF::NoIntersection)
          {
            QPointF mid = QLineF(startPoint, endPoint).pointAt(0.5);
            if(startOrientation == connectable_item::NORTH || startOrientation == connectable_item::SOUTH)
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
}
