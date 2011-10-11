#include <pnete/ui/graph/style.hpp>
#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/size.hpp>

#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QTransform>

#include <list>
#include <cmath>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace cap
        {
          namespace data
          {
            class data : public std::list<QPointF>
            {
            protected:
              void push (const qreal& x, const qreal& y)
              {
                push_back (QPointF (x, y));
              }

            public:
              data () : std::list<QPointF>() {}
            };

            class out : public data
            {
            public:
              out () : data ()
              {
                push (0.0                , -(size::port::height() / 2.0));
                push (size::cap::length(),                          0.0 );
                push (0.0                ,  (size::port::height() / 2.0));
              }
            };

            class in : public data
            {
            public:
              in () : data ()
              {
                push (0.0                , -(size::port::height() / 2.0));
                push (size::cap::length(), -(size::port::height() / 2.0));
                push (0.0                ,                          0.0 );
                push (size::cap::length(),  (size::port::height() / 2.0));
                push (0.0                ,  (size::port::height() / 2.0));
              }
            };
          } // namespace data

          template<typename IT>
          void add ( QPainterPath* path
                   , const bool& middle
                   , const QPointF& offset
                   , const qreal& rotation
                   , IT pos
                   , const IT& end
                   )
          {
            QTransform transformation;
            transformation.translate (offset.x(), offset.y())
              .rotate (rotation);

            const qreal shift (middle ? (size::port::height() / 2.0) : 0.0);

            for (; pos != end; ++pos)
              {
                path->lineTo
                  (transformation.map (QPointF (pos->x(), pos->y() + shift)));
              }
          }
        } // namespace cap

        void add_outgoing_cap ( QPainterPath* path
                              , bool middle
                              , QPointF offset = QPointF()
                              , qreal rotation = 0.0
                              )
        {
          static const cap::data::out out;

          cap::add (path, middle, offset, rotation, out.begin(), out.end());
        }

        void add_outgoing_cap ( QPolygonF* poly
                              , QPointF offset = QPointF()
                              , qreal rotation = 0.0
                              )
        {
          QPainterPath path;
          add_outgoing_cap (&path, false, offset, rotation);
          *poly = poly->united (path.toFillPolygon());
        }

        void add_incoming_cap ( QPainterPath* path
                              , bool middle
                              , QPointF offset = QPointF()
                              , qreal rotation = 0.0
                              )
        {
          static const cap::data::in in;

          cap::add (path, middle, offset, rotation, in.begin(), in.end());
        }

        void add_incoming_cap ( QPolygonF* poly
                              , QPointF offset = QPointF()
                              , qreal rotation = 0.0
                              )
        {
          QPainterPath path;
          add_incoming_cap (&path, false, offset, rotation);
          *poly = poly->united (path.toFillPolygon());
        }

        namespace port
        {
          static qreal angle (const orientation::type& o)
          {
            switch (o)
              {
              case orientation::NORTH: return -90.0;
              case orientation::EAST: return 0.0;
              case orientation::SOUTH: return 90.0;
              case orientation::WEST: return 180.0;
              default:
                throw std::runtime_error ("angle (NON_ORIENTATION)!?");
              }
          }

          QPainterPath item::shape () const
          {
            const qreal lengthHalf (length() / 2.0); // hardcoded constant

            QPolygonF poly;
            const qreal y (size::port::height() / 2.0);
            poly << QPointF ( lengthHalf - size::cap::length(),  y)
                 << QPointF (-lengthHalf                      ,  y)
                 << QPointF (-lengthHalf                      , -y)
                 << QPointF ( lengthHalf - size::cap::length(), -y)
              ;

            if (direction() == connectable_item::IN)
              {
                add_incoming_cap (&poly, QPointF (lengthHalf - size::cap::length(), 0.0));   // hardcoded constant
              }
            else
              {
                add_outgoing_cap (&poly, QPointF (lengthHalf - size::cap::length(), 0.0));
              }

            poly = QTransform().rotate (angle (orientation())).map (poly);

            QPainterPath path;
            path.addPolygon (poly);
            path.closeSubpath();

            return path;
          }

          QRectF item::bounding_rect (bool cap, int cap_factor) const
          {
            const qreal addition ( cap
                                 ? 0.0
                                 : size::cap::length() * cap_factor
                                 );
            const qreal lengthHalf ((length() - addition) / 2.0);                                  // hardcoded constant

            switch (orientation())
              {
              case orientation::NORTH:
                return QRectF ( -(size::port::height() / 2.0)
                              , -lengthHalf + addition
                              , size::port::height()
                              , length() - addition
                              );

              case orientation::SOUTH:
                return QRectF ( -(size::port::height() / 2.0)
                              , -lengthHalf
                              , size::port::height()
                              , length() - addition
                              );

              case orientation::EAST:
                return QRectF ( -lengthHalf
                              , -(size::port::height() / 2.0)
                              , length() - addition
                              , size::port::height()
                              );

              case orientation::WEST:
                return QRectF ( -lengthHalf + addition
                              , -(size::port::height() / 2.0)
                              , length() - addition
                              , size::port::height()
                              );
              default:
                throw std::runtime_error("invalid port direction!");
              }
          }

          QRectF item::boundingRect () const
          {
            return bounding_rect ();
          }

          QColor queryColorForType (const QString& type)
          {
            //! \note Colors shamelessly stolen from PSPro.
            //! \todo Maybe also do a gradient? Maybe looks awesome.
            if (type.startsWith ("seismic"))
              {
                return QColor (0, 130, 250);                                           // hardcoded constant
              }
            else if (type.startsWith ("velocity"))
              {
                return QColor (248, 248, 6);                                           // hardcoded constant
              }
            else
              {
                return QColor (255, 255, 255);                                         // hardcoded constant
              }
          }

          void item::paint ( QPainter *painter
                           , const QStyleOptionGraphicsItem *
                           , QWidget *
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
            painter->setBrush (QBrush ( queryColorForType (we_type())
                                      , Qt::SolidPattern
                                      )
                              );
            painter->drawPath (shape());

            painter->setPen (QPen (QBrush (Qt::black), 1.0));
            painter->setBackgroundMode (Qt::TransparentMode);

            const QRectF area (bounding_rect (false, 1));

            if ( orientation() == orientation::NORTH
               || orientation() == orientation::SOUTH
               )
              {
                const qreal degrees (90.0);                                                 // hardcoded constant

                painter->save();
                painter->rotate (degrees);
                painter->drawText ( QTransform().rotate(-degrees).mapRect (area)
                                  , Qt::AlignCenter
                                  , name()
                                  );
                painter->restore();
              }
            else
              {
                painter->drawText(area, Qt::AlignCenter, name());
              }
          }
        } // namespace port

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

            add_outgoing_cap ( &path
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

            add_incoming_cap ( &path
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

        namespace transition
        {
          QPainterPath item::shape (const QSizeF& size) const
          {
            QPainterPath path;
            path.addRoundRect (bounding_rect (size), 20); // hardcoded constant
            return path;
          }
          QPainterPath item::shape () const
          {
            return shape (_size);
          }
          QRectF item::bounding_rect (const QSizeF& size) const
          {
            const QSizeF half_size (size / 2.0);
            return QRectF ( -half_size.width()
                          , -half_size.height()
                          , size.width()
                          , size.height()
                          );
          }
          QRectF item::boundingRect () const
          {
            return bounding_rect (_size);
          }

          void item::paint ( QPainter* painter
                           , const QStyleOptionGraphicsItem *option
                           , QWidget *widget
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
            painter->drawPath (shape());

            painter->setPen (QPen (QBrush (Qt::black), 1.0));
            painter->setBackgroundMode (Qt::TransparentMode);

            QRectF rect (boundingRect());
            rect.setWidth (rect.width() - size::port::width());
            rect.setHeight (rect.height() - size::port::width());
            rect.translate ( size::port::height() / 2.0
                           , size::port::height() / 2.0
                           );

            painter->drawText ( rect
                              , Qt::AlignCenter | Qt::TextWordWrap
                              , name()
                              );
          }
        } // namespace transition

        namespace style
        {
          namespace raster
          {
            qreal snap (const qreal& x)
            {
              return size::raster() * std::ceil (x / size::raster());
            }

            QPointF snap (const QPointF& pos)
            {
              return QPointF (snap (pos.x()), snap (pos.y()));
            }
          } // namespace raster
        }
      }
    }
  }
}
