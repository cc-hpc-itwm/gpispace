
#include "Connection.hpp"
#include "ConnectableItem.hpp"
#include "Style.hpp"
#include "Scene.hpp"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      Connection::Connection(ConnectableItem* start, ConnectableItem* end, ConnectableItem* parent)
      : QGraphicsItem(parent),
      _start(NULL),
      _end(NULL),
      _highlighted(false)
      {
        setStart(start);
        setEnd(end);
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
      }
      
      const ConnectableItem* Connection::start() const
      {
        return _start;
      }
      const ConnectableItem* Connection::end() const
      {
        return _end;
      }
      const bool Connection::isLookingForStart() const
      {
        return _start == NULL;
      }
      const bool Connection::isLookingForEnd() const
      {
        return _end == NULL;
      }
      
      const QPointF Connection::startPosition() const
      {
        if(!qobject_cast<Scene*>(scene())) return QPointF();
        return ( _start ? QPointF(_start->scenePos()) : qobject_cast<Scene*>(scene())->mousePosition() ) - mapToScene(0.0f, 0.0f);
      }
      const QPointF Connection::endPosition() const
      {
        if(!qobject_cast<Scene*>(scene())) return QPointF();
        return ( _end ? QPointF(_end->scenePos()) : qobject_cast<Scene*>(scene())->mousePosition() ) - mapToScene(0.0f, 0.0f);
      }
      
      QPainterPath Connection::shape() const
      {
        if(_start || _end)
        {          
          return Style::connectionShape(this);
        }
        else
        {
          return QPainterPath();
        }
      }

      QRectF Connection::boundingRect() const
      { 
        if(_start || _end)
        {
          return Style::connectionBoundingRect(this);
        }
        else
        {
          return QRectF();
        }
      }
      
      void Connection::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
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
    }
  }
}
