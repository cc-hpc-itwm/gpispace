// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/association.hpp>

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
        association::association
          ( connectable_item* start
          , connectable_item* end
          )
            : base_item()
            , _start (start)
            , _end (end)
            , _fixed_points()
            , _dragged_point (boost::none)
        {
          start->add_association (this);
          end->add_association (this);

          setZValue (-1);                                                          // hardcoded constant
          set_just_pos_but_not_in_property (0.0, 0.0);
        }
        association::~association()
        {
          _start->remove_association (this);
          _end->remove_association (this);
        }

        connectable_item* association::start() const
        {
          return _start;
        }
        connectable_item* association::end() const
        {
          return _end;
        }

        const QList<QPointF>& association::fixed_points() const
        {
          return _fixed_points;
        }

        const QList<QPointF>&
        association::fixed_points (const QList<QPointF>& fixed_points_)
        {
          return _fixed_points = fixed_points_;
        }

        QList<QPointF> association::all_points() const
        {
          QList<QPointF> result;
          result << start()->scenePos() << fixed_points() << end()->scenePos();
          return result;
        }

        void association::invert()
        {
          const int size (_fixed_points.size());
          for (int k (0); k < (size / 2); ++k)
          {
            _fixed_points.swap (k, size - (1 + k));
          }
          std::swap (_start, _end);
        }

        QPainterPath association::shape () const
        {
          return style::association::shape (all_points());
        }

        void association::paint ( QPainter* painter
                                    , const QStyleOptionGraphicsItem*
                                    , QWidget*
                                    )
        {
          style::draw_shape (this, painter);
        }

        void association::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          if (event->modifiers() == Qt::ControlModifier)
          {
            _dragged_point = boost::none;
            boost::optional<std::pair<int,qreal> > nearest_point (boost::none);

            {
              int i (0);
              for ( QList<QPointF>::const_iterator it (fixed_points().begin())
                  ; it != fixed_points().end()
                  ; ++it, ++i
                  )
              {
                static const qreal still_clicking_point_dist (30.0);
                const qreal dist (QLineF (event->pos(), *it).length());
                if ( dist < still_clicking_point_dist
                   && (!nearest_point || nearest_point->second < dist)
                   )
                {
                  nearest_point = std::make_pair (i, dist);
                }
              }
            }

            if (!nearest_point)
            {
              QList<QPointF> points (all_points());
              int i (0);
              for ( QList<QPointF>::const_iterator first (points.begin())
                  , second (points.begin() + 1)
                  ; second != points.end()
                  ; ++first, ++second, ++i
                  )
              {
                const QLineF segment (*first, *second);
                // get a unit vector from event->pos() with angle like normal
                QLineF fake_normal ( QLineF ( event->pos()
                                            , event->pos() + QPointF (1., 0.)
                                            )
                                   );
                fake_normal.setAngle (segment.normalVector().angle());
                QPointF intersection;
                // get point where both intersect
                if ( fake_normal.intersect (segment, &intersection)
                   != QLineF::NoIntersection
                   )
                {
                  // scale normal from [0,1] to [-dist - 1, dist + 1]
                  fake_normal.setLength
                    (QLineF (event->pos(), intersection).length() + 1.);
                  fake_normal.setPoints (fake_normal.p2(), fake_normal.p1());
                  fake_normal.setLength (2 * fake_normal.length());
                  // check, if they intersect within the segment
                  if ( fake_normal.intersect (segment, NULL)
                     == QLineF::BoundedIntersection
                     )
                  {
                    _fixed_points.insert (i, intersection);
                    _dragged_point = i;
                    break;
                  }
                }
              }
            }
            else
            {
              _dragged_point = nearest_point->first;
            }
          }

          if (_dragged_point)
          {
            mode_push (mode::DRAG);
          }
          else
          {
            base_item::mousePressEvent (event);
          }
        }

        void association::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
        {
          if (mode() == mode::DRAG)
          {
            mode_pop();
            _dragged_point = boost::none;
          }
          else
          {
            base_item::mouseReleaseEvent (event);
          }
        }
        void association::mouseMoveEvent (QGraphicsSceneMouseEvent* event)
        {
          if (mode() == mode::DRAG)
          {
            const QRectF old (boundingRect());
            _fixed_points[*_dragged_point] = event->pos();
            scene()->update (old.united (boundingRect()));
          }
          else
          {
            base_item::mouseMoveEvent (event);
          }
        }
      }
    }
  }
}
