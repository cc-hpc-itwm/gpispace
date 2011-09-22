#include <pnete/ui/graph/style.hpp>
#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/transition.hpp>

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
        static const qreal rasterSize (10.0);                                     // hardcoded constant

        static const qreal portHeight (20.0);                                     // hardcoded constant
        static const qreal portHeightHalf (portHeight / 2.0);                     // hardcoded constant
        static const qreal defaultPortWidth (60.0);                               // hardcoded constant

        static const qreal capLength (10.0);                                      // hardcoded constant

        // defined from the center of the end of the line to attach to. clockwise
        static const QPointF outgoingCap[] = { QPointF(0.0, -portHeightHalf)
                                             , QPointF(capLength, 0.0)
                                             , QPointF(0.0, portHeightHalf)
                                             };
        static const QPointF ingoingCap[] = { QPointF(0.0, -portHeightHalf)
                                            , QPointF(capLength, -portHeightHalf)
                                            , QPointF(0.0, 0.0)
                                            , QPointF(capLength, portHeightHalf)
                                            , QPointF(0.0, portHeightHalf)
                                            };

        qreal style::portCapLength()
        {
          return capLength;
        }
        qreal style::portDefaultWidth()
        {
          return defaultPortWidth;
        }
        qreal style::portDefaultHeight()
        {
          return portHeight;
        }

        void addOutgoingCap ( QPainterPath* path
                            , bool middle
                            , QPointF offset = QPointF()
                            , qreal rotation = 0.0
                            )
        {
          QTransform transformation;
          transformation.rotate(rotation);

          for(size_t i (0); i < sizeof(outgoingCap) / sizeof(QPointF); ++i)
          {
            path->lineTo ( transformation.map (QPointF ( outgoingCap[i].x()
                                                       , outgoingCap[i].y()
                                                       + ( middle
                                                         ? portHeightHalf
                                                         : 0.0
                                                         )
                                                       )
                                              )
                         + offset
                         );
          }
        }

        void addOutgoingCap ( QPolygonF* poly
                            , QPointF offset = QPointF()
                            , qreal rotation = 0.0
                            )
        {
          QPainterPath path;
          addOutgoingCap (&path, false, offset, rotation);
          *poly = poly->united (path.toFillPolygon());
        }

        void addIngoingCap ( QPainterPath* path
                           , bool middle
                           , QPointF offset = QPointF()
                           , qreal rotation = 0.0
                           )
        {
          QTransform transformation;
          transformation.rotate(rotation);

          for(size_t i (0); i < sizeof(ingoingCap) / sizeof(QPointF); ++i)
          {
            path->lineTo ( transformation.map ( QPointF ( ingoingCap[i].x()
                                                        , ingoingCap[i].y()
                                                        + ( middle
                                                          ? portHeightHalf
                                                          : 0.0
                                                          )
                                                        )
                                              )
                         + offset
                         );
          }
        }

        void addIngoingCap ( QPolygonF* poly
                           , QPointF offset = QPointF()
                           , qreal rotation = 0.0
                           )
        {
          QPainterPath path;
          addIngoingCap (&path, false, offset, rotation);
          *poly = poly->united (path.toFillPolygon());
        }

        static qreal angle (const port::ORIENTATION& o)
        {
          switch (o)
          {
          case port::NORTH: return -90.0;
          case port::EAST: return 0.0;
          case port::SOUTH: return 90.0;
          case port::WEST: return 180.0;
          default:
            throw std::runtime_error ("angle (NON_ORIENTATION)!?");
          }
        }

        QPainterPath style::portShape (const port* port)
        {
          const qreal& length (port->length());
          const qreal lengthHalf (length / 2.0);                                  // hardcoded constant

          QTransform rotation;
          rotation.rotate (angle (port->orientation()));

          QPainterPath path;

          QPolygonF poly;
          poly << QPointF (lengthHalf - portCapLength(), portHeightHalf)
               << QPointF (-lengthHalf, portHeightHalf)
               << QPointF (-lengthHalf, -portHeightHalf)
               << QPointF (lengthHalf - portCapLength(), -portHeightHalf);

          if (port->direction() == connectable_item::IN)
          {
            addIngoingCap (&poly, QPointF (lengthHalf - portCapLength(), 0.0));   // hardcoded constant
          }
          else
          {
            addOutgoingCap (&poly, QPointF (lengthHalf - portCapLength(), 0.0));
          }

          poly = rotation.map (poly);

          path.addPolygon (poly);
          path.closeSubpath();

          return path;
        }

        QRectF style::portBoundingRect ( const port* port
                                       , bool withCap
                                       , int capFactor
                                       )
        {
          const qreal addition ( withCap
                               ? 0.0
                               : portCapLength() * capFactor
                               );
          const qreal length (port->length() - addition);
          const qreal lengthHalf (length / 2.0);                                  // hardcoded constant

          switch (port->orientation())
          {
          case port::NORTH:
            return QRectF ( -portHeightHalf
                          , -lengthHalf + addition
                          , portHeight
                          , length
                          );

          case port::SOUTH:
            return QRectF ( -portHeightHalf
                          , -lengthHalf
                          , portHeight
                          , length
                          );

          case port::EAST:
            return QRectF ( -lengthHalf
                          , -portHeightHalf
                          , length
                          , portHeight
                          );

          case port::WEST:
            return QRectF ( -lengthHalf + addition
                          , -portHeightHalf
                          , length
                          , portHeight
                          );
          }
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

        void style::portPaint (QPainter *painter, const port* port)
        {
          // hardcoded constants
          painter->setPen (QPen (QBrush ( port->isSelected()
                                        ? Qt::red
                                        : Qt::black
                                        )
                                , 2.0
                                )
                          );
          painter->setBackgroundMode (Qt::OpaqueMode);
          painter->setBrush (QBrush ( queryColorForType (port->we_type())
                                    , Qt::SolidPattern
                                    )
                            );
          painter->drawPath (portShape (port));

          painter->setPen (QPen (QBrush (Qt::black), 1.0));
          painter->setBackgroundMode (Qt::TransparentMode);

          const QRectF area (portBoundingRect (port, false, 1));

          if ( port->orientation() == port::NORTH
            || port->orientation() == port::SOUTH
             )
          {
            const qreal degrees (90.0);                                                 // hardcoded constant

            QTransform antirotation;
            antirotation.rotate (-degrees);

            painter->save();
            painter->rotate (degrees);
            painter->drawText ( antirotation.mapRect (area)
                              , Qt::AlignCenter
                              , port->name()
                              );
            painter->restore();
          }
          else
          {
            painter->drawText(area, Qt::AlignCenter, port->name());
          }
        }

        QPainterPath style::connectionShape (const connection* connection)
        {
          const connectable_item* startItem (connection->start());
          const connectable_item* endItem (connection->end());
          if(!startItem && !endItem)
          {
            return QPainterPath();
          }

          const QPointF start ( startItem
                              ? startItem->scenePos()
                              : connection->scene()->mouse_position()
                              );
          const QPointF end ( endItem
                            ? endItem->scenePos()
                            : connection->scene()->mouse_position()
                            );
          const QList<QPointF>& fixed_points (connection->fixed_points());

          //! \todo indicate connection with blob?
          if(start == end)
          {
            return QPainterPath();
          }

          QPainterPath path;
          path.setFillRule (Qt::WindingFill);

          QList<QPointF> allPoints;
          allPoints.push_back (start);
          foreach (QPointF point, fixed_points)
          {
            allPoints.push_back (point);
          }
          allPoints.push_back (end);

          QList<QLineF> linesForward;
          QList<QLineF> linesBackward;

          for ( QList<QPointF>::const_iterator first (allPoints.begin())
              , second (allPoints.begin() + 1)
              ; second != allPoints.end()
              ; ++first
              , ++second
              )
          {
            const qreal dummyLength (QLineF (*first, *second).length());

            QTransform transformation;
            transformation.translate (first->x(), first->y());
            transformation.rotate (-QLineF (*first, *second).angle());

            linesForward.push_back
              ( transformation.map ( QLineF ( QPointF ( 0.0
                                                      , -portHeightHalf
                                                      )
                                            , QPointF ( dummyLength
                                                      , -portHeightHalf
                                                      )
                                            )
                                   )
              );
            linesBackward.push_front
              ( transformation.map ( QLineF ( QPointF ( 0.0
                                                      , portHeightHalf
                                                      )
                                            , QPointF ( dummyLength
                                                      , portHeightHalf
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

          addOutgoingCap ( &path
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

          addIngoingCap ( &path
                        , true
                        , linesBackward.last().p1()
                        , 180.0 - linesBackward.last().angle()
                        );

          path.lineTo (linesForward.first().p1());

          return path;
        }
        QRectF style::connectionBoundingRect (const connection* connection)
        {
          return connectionShape (connection).boundingRect();
        }
        void style::connectionPaint ( QPainter* painter
                                    , const connection* connection
                                    )
        {
          // hardcoded constants
          painter->setPen (QPen (QBrush ( connection->isSelected()
                                        ? Qt::red
                                        : Qt::black
                                        )
                                , 2.0
                                )
                          );
          painter->setBackgroundMode (Qt::OpaqueMode);
          painter->setBrush (QBrush (Qt::white, Qt::SolidPattern));
          painter->drawPath (connectionShape (connection));
        }

        QPainterPath style::transitionShape (const QSizeF& size)
        {
          QPainterPath path;
          path.addRoundRect (transitionBoundingRect (size), 20);                    // hardcoded constant
          return path;
        }
        QRectF style::transitionBoundingRect (const QSizeF& size)
        {
          return QRectF (0.0, 0.0, size.width(), size.height());
        }
        void style::transitionPaint ( QPainter* painter
                                    , const transition* transition
                                    )
        {
          // hardcoded constants
          painter->setPen (QPen (QBrush ( transition->isSelected()
                                        ? Qt::red
                                        : Qt::black
                                        )
                                , 2.0
                                )
                          );
          painter->setBackgroundMode (Qt::OpaqueMode);
          painter->setBrush (QBrush (Qt::white, Qt::SolidPattern));
          painter->drawPath (transition->shape());

          painter->setPen (QPen (QBrush (Qt::black), 1.0));
          painter->setBackgroundMode (Qt::TransparentMode);

          QRectF boundingRect (transition->boundingRect());
          boundingRect.setWidth (boundingRect.width() - portDefaultWidth());
          boundingRect.setHeight (boundingRect.height() - portDefaultWidth());
          boundingRect.translate ( portDefaultWidth() / 2.0
                                 , portDefaultWidth() / 2.0
                                 );

          painter->drawText ( boundingRect
                            , Qt::AlignCenter | Qt::TextWordWrap
                            , transition->name()
                            );
        }

        qreal style::raster()
        {
          return rasterSize;
        }
        QPointF style::snapToRaster (const QPointF& pos)
        {
          const qreal raster (style::raster());
          return QPointF ( raster * static_cast<int> (pos.x() / raster)
                         , raster * static_cast<int> (pos.y() / raster)
                         );
        }
      }
    }
  }
}
