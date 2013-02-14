// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/style/association.hpp>

#include <pnete/ui/graph/style/cap.hpp>
#include <pnete/ui/graph/style/size.hpp>

#include <QPainterPath>
#include <QPointF>
#include <QTransform>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace style
        {
          namespace association
          {
            namespace
            {
              QPainterPath shape_impl
                (const QList<QPointF>& points, bool add_cap)
              {
                if(points.size() <= 1 || points.first() == points.last())
                {
                  return QPainterPath();
                }

                QPainterPath path;
                path.setFillRule (Qt::WindingFill);

                QList<QLineF> linesForward;
                QList<QLineF> linesBackward;

                for ( QList<QPointF>::const_iterator first (points.begin())
                    , second (points.begin() + 1)
                    ; second != points.end()
                    ; ++first, ++second
                    )
                {
                  const qreal dummyLength (QLineF (*first, *second).length());

                  QTransform transformation;
                  transformation.translate (first->x(), first->y())
                    .rotate (-QLineF (*first, *second).angle());

                  const qreal half_height (size::port::height() / 2.0);

                  linesForward.push_back
                    ( transformation.map ( QLineF ( QPointF (0.0, -half_height)
                                                  , QPointF ( dummyLength
                                                            , -half_height
                                                            )
                                                  )
                                         )
                    );
                  linesBackward.push_front
                    ( transformation.map ( QLineF ( QPointF (0.0, half_height)
                                                  , QPointF ( dummyLength
                                                            , half_height
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

                if (add_cap)
                {
                  cap::add_outgoing ( &path
                                    , true
                                    , linesForward.last().p2()
                                  , -linesForward.last().angle()
                                    );
                }

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

                if (add_cap)
                {
                  cap::add_incoming ( &path
                                    , true
                                    , linesBackward.last().p1()
                                    , 180.0 - linesBackward.last().angle()
                                    );
                }

                path.lineTo (linesForward.first().p1());

                return path;
              }
            }

            QPainterPath shape (const QList<QPointF>& points)
            {
              return shape_impl (points, true);
            }

            QPainterPath shape_no_cap (const QList<QPointF>& points)
            {
              return shape_impl (points, false);
            }
          }
        }
      }
    }
  }
}
