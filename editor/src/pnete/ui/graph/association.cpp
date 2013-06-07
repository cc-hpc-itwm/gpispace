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
        {
          start->add_association (this);
          end->add_association (this);

          setZValue (-1);                                                          // hardcoded constant
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

        QList<QPointF> association::all_points() const
        {
          QList<QPointF> result;
          result << start()->scenePos() << end()->scenePos();
          return result;
        }

        void association::invert()
        {
          std::swap (_start, _end);
        }

        QPainterPath association::shape() const
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

        bool association::is_movable() const
        {
          return false;
        }
      }
    }
  }
}
