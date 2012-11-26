// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connection.hpp>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/style/association.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        connection_item::connection_item
          ( const boost::optional<data::handle::connect>& handle
          , bool read
          )
            : base_item()
            , _handle (handle)
            , _start (NULL)
            , _end (NULL)
            , _fixed_points()
            , _read (read)
        {
          setZValue (-1);                                                          // hardcoded constant
        }
        connection_item::~connection_item()
        {
          start (NULL);
          end (NULL);
        }

        connectable_item* connection_item::start() const
        {
          return _start;
        }
        connectable_item* connection_item::start (connectable_item* start_)
        {
          if (_start)
          {
            _start->remove_connection (this);
          }
          _start = start_;
          if (_start)
          {
            _start->add_connection (this);
          }
          return _start;
        }
        connectable_item* connection_item::end() const
        {
          return _end;
        }
        connectable_item* connection_item::end (connectable_item* end_)
        {
          if (_end)
          {
            _end->remove_connection (this);
          }
          _end = end_;
          if (_end)
          {
            _end->add_connection (this);
          }
          return _end;
        }

        connectable_item* connection_item::non_free_side() const
        {
          if (_start && _end)
          {
            throw std::runtime_error ( "can't get a non_free side as both "
                                     "sides are connected."
                                     );
          }
          if (!_start && !_end)
          {
            throw std::runtime_error ( "can't get a non_free side as both "
                                     "sides are not connected."
                                     );
          }
          return _start ? _start : _end;
        }
        connectable_item* connection_item::free_side(connectable_item* item)
        {
          if (_start && _end)
          {
            throw std::runtime_error ( "can't connect free side, as there "
                                     "is none."
                                     );
          }
          if (!_start && !_end)
          {
            throw std::runtime_error ("can't connect free side, as both are.");
          }
          return _end ? start (item) : end (item);
        }

        const QList<QPointF>& connection_item::fixed_points() const
        {
          return _fixed_points;
        }

        const QList<QPointF>&
        connection_item::fixed_points (const QList<QPointF>& fixed_points_)
        {
          return _fixed_points = fixed_points_;
        }

        const bool& connection_item::read() const
        {
          return _read;
        }
        const bool& connection_item::read (const bool& read_)
        {
          return _read = read_;
        }

        //! \todo this is broken
        QPainterPath connection_item::shape () const
        {
          QList<QPointF> allPoints;
          allPoints.push_back ( start()
                              ? start()->scenePos()
                              : scene()->mouse_position()
                              );
          foreach (QPointF point, fixed_points())
          {
            allPoints.push_back (point);
          }
          allPoints.push_back ( end()
                              ? end()->scenePos()
                              : scene()->mouse_position()
                              );

          return association::shape (allPoints);
        }

        void connection_item::paint ( QPainter* painter
                                    , const QStyleOptionGraphicsItem*
                                    , QWidget*
                                    )
        {
          style::draw_shape (this, painter);
        }

        void connection_item::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          if (event->modifiers() == Qt::ControlModifier)
          {
            event->ignore();
          }
        }
      }
    }
  }
}
