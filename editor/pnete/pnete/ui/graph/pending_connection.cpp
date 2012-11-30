// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/pending_connection.hpp>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/style/association.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        pending_connection::pending_connection
          ( const connectable_item* fixed_end
          , const QPointF& open_end
          , base_item* parent
          )
            : base_item (parent)
            , _fixed_end (fixed_end)
            , _open_end (open_end)
        {
          setZValue (-1);
        }

        QPainterPath pending_connection::shape() const
        {
          QList<QPointF> points;
          points.push_back
            ( _fixed_end->direction() == connectable::direction::OUT
            ? _fixed_end->scenePos()
            : _open_end
            );
          //! \todo automatic orthogonal lines
          points.push_back
            ( _fixed_end->direction() == connectable::direction::OUT
            ? _open_end
            : _fixed_end->scenePos()
            );

          return style::association::shape (points);
        }
        void pending_connection::paint
          (QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
        {
          style::draw_shape (this, painter);
        }

        void pending_connection::open_end (const QPointF& open_end)
        {
          _open_end = open_end;
        }

        const connectable_item* pending_connection::fixed_end() const
        {
          return _fixed_end;
        }
      }
    }
  }
}
