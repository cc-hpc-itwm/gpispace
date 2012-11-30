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
          //! \todo Add ability to set control points.
          if (event->modifiers() == Qt::ControlModifier)
          {
            event->ignore();
          }
          else
          {
            base_item::mousePressEvent (event);
          }
        }
      }
    }
  }
}
