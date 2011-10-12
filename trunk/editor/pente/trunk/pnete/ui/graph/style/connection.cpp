// {bernd.loerwald,mirko.rahn}@item.fraunhofer.de

#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/connectable_item.hpp>

#include <pnete/ui/graph/size.hpp>

#include <pnete/ui/graph/style/cap.hpp>

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QTransform>

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

          QRectF item::boundingRect () const
          {
            return shape ().boundingRect();
          }
          void item::paint ( QPainter* painter
                           , const QStyleOptionGraphicsItem*
                           , QWidget*
                           )
          {
            // hardcoded constants
            painter->setPen (QPen (QBrush ( isSelected()
                                          ? Qt::red
                                          : Qt::black
                                          )
                                  , 2.0
                                  )
                            );
            painter->setBackgroundMode (Qt::OpaqueMode);
            painter->setBrush (QBrush (Qt::white, Qt::SolidPattern));
            painter->drawPath (shape ());
          }
        } // namespace connection
      }
    }
  }
}
