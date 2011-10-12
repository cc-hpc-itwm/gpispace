// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/size.hpp>
#include <pnete/ui/graph/style/cap.hpp>

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
                cap::add_incoming (&poly, QPointF (lengthHalf - size::cap::length(), 0.0));   // hardcoded constant
              }
            else
              {
                cap::add_outgoing (&poly, QPointF (lengthHalf - size::cap::length(), 0.0));
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
      }
    }
  }
}
