// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connection.hpp>

#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include <pnete/ui/graph/connectable_item.hpp>

#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/style/size.hpp>
#include <pnete/ui/graph/style/cap.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace connection
        {
          item::item (bool read)
            : graph::item()
            , _start (NULL)
            , _end (NULL)
            , _fixed_points()
            , _read (read)
          {
            setZValue (-1);                                                          // hardcoded constant
          }
          item::~item()
          {
            start (NULL);
            end (NULL);
          }

          connectable_item* item::start() const
          {
            return _start;
          }
          connectable_item* item::start (connectable_item* start_)
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
          connectable_item* item::end() const
          {
            return _end;
          }
          connectable_item* item::end (connectable_item* end_)
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

          connectable_item* item::non_free_side() const
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
          connectable_item* item::free_side(connectable_item* item)
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

          const QList<QPointF>& item::fixed_points() const
          {
            return _fixed_points;
          }

          const QList<QPointF>&
          item::fixed_points (const QList<QPointF>& fixed_points_)
          {
            return _fixed_points = fixed_points_;
          }

          const bool& item::read() const
          {
            return _read;
          }
          const bool& item::read (const bool& read_)
          {
            return _read = read_;
          }

          //! \todo this is broken
          QPainterPath item::shape () const
          {
            if(!start() && !end())
              {
                return QPainterPath();
              }

            const QPointF start_point ( start()
                                      ? start()->scenePos()
                                      : scene()->mouse_position()
                                      );
            const QPointF end_point ( end()
                                    ? end()->scenePos()
                                    : scene()->mouse_position()
                                    );
            //! \todo indicate connection with blob?
            if(start_point == end_point)
              {
                return QPainterPath();
              }

            QPainterPath path;
            path.setFillRule (Qt::WindingFill);

            QList<QPointF> allPoints;
            allPoints.push_back (start_point);
            foreach (QPointF point, fixed_points())
              {
                allPoints.push_back (point);
              }
            allPoints.push_back (end_point);

            QList<QLineF> linesForward;
            QList<QLineF> linesBackward;

            for ( QList<QPointF>::const_iterator first (allPoints.begin())
                    , second (allPoints.begin() + 1)
                ; second != allPoints.end()
                ; ++first, ++second
                )
              {
                const qreal dummyLength (QLineF (*first, *second).length());

                QTransform transformation;
                transformation.translate (first->x(), first->y())
                  .rotate (-QLineF (*first, *second).angle());

                linesForward.push_back
                  ( transformation.map ( QLineF ( QPointF ( 0.0
                                                          , -(size::port::height() / 2.0)
                                                          )
                                                , QPointF ( dummyLength
                                                          , -(size::port::height() / 2.0)
                                                          )
                                                )
                                       )
                  );
                linesBackward.push_front
                  ( transformation.map ( QLineF ( QPointF ( 0.0
                                                          , (size::port::height() / 2.0)
                                                          )
                                                , QPointF ( dummyLength
                                                          , (size::port::height() / 2.0)
                                                          )
                                                )
                                       )
                  );
              }

            path.moveTo (linesForward.first().p1());

            QPointF intersection;

            for ( QList<QLineF>::iterator line (linesForward.begin())
                ; line != linesForward.end()
                ; ++line
                )
              {
                QPointF target (line->p2());

                QList<QLineF>::iterator nextLine (line + 1);
                if ( nextLine != linesForward.end()
                   && line->intersect (*nextLine, &intersection)
                   != QLineF::NoIntersection
                   )
                  {
                    target = intersection;
                  }

                path.lineTo (target);
              }

            cap::add_outgoing ( &path
                              , true
                              , linesForward.last().p2()
                              , -linesForward.last().angle()
                              );

            path.lineTo (linesBackward.first().p2());

            for ( QList<QLineF>::iterator line (linesBackward.begin())
                ; line != linesBackward.end()
                ; ++line
                )
              {
                QPointF target (line->p1());

                QList<QLineF>::iterator nextLine (line + 1);
                if ( nextLine != linesBackward.end()
                   && line->intersect (*nextLine, &intersection)
                   != QLineF::NoIntersection
                   )
                  {
                    target = intersection;
                  }

                path.lineTo (target);
              }

            cap::add_incoming ( &path
                              , true
                              , linesBackward.last().p1()
                              , 180.0 - linesBackward.last().angle()
                              );

            path.lineTo (linesForward.first().p1());

            return path;
          }

          void item::paint ( QPainter* painter
                           , const QStyleOptionGraphicsItem*
                           , QWidget*
                           )
          {
            style::draw_shape (this, painter);
          }

          void item::mousePressEvent (QGraphicsSceneMouseEvent* event)
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
}
