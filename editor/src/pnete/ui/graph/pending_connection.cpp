// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/pending_connection.hpp>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/style/association.hpp>
#include <xml/parse/type/port.hpp>

#include <pnete/ui/graph/style/isc13.hpp>

#include <util/qt/cast.hpp>


namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace
        {
          boost::optional<QBrush> color_if_type
            (const base_item* item, const QBrush& c, const QString& type)
          {
            return boost::make_optional
              ( fhg::util::qt::throwing_qobject_cast<const port_item*>
              (fhg::util::qt::throwing_qobject_cast<const pending_connection*>
                (item)->fixed_end())->handle().type() == type
              , c
              );
          }
        }

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

          if (const port_item* fix = qobject_cast<const port_item*> (_fixed_end))
          {
            if (fix->handle().is_tunnel())
            {
              _style.push<Qt::PenStyle> ("border_style", Qt::DotLine);
            }
          }

          style::isc13::add_colors_for_types (&_style, color_if_type);
        }

        QPainterPath pending_connection::shape() const
        {
          QList<QPointF> points;

          //! \todo automatic orthogonal lines
          if (const port_item* fix = qobject_cast<const port_item*> (_fixed_end))
          {
            if (fix->handle().is_input())
            {
              points.push_back (_open_end);
              points.push_back (_fixed_end->scenePos());
            }
            else if (fix->handle().is_output())
            {
              points.push_back (_fixed_end->scenePos());
              points.push_back (_open_end);
            }
            else if (fix->handle().is_tunnel())
            {
              points.push_back (_fixed_end->scenePos());
              points.push_back (_open_end);

              return style::association::shape_no_cap (points);
            }
          }
          else
          {
            points.push_back (_fixed_end->scenePos());
            points.push_back (_open_end);
          }

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
