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

          style::isc13::add_colors_for_types (&_style, color_if_type);
        }

        namespace
        {
          we::type::PortDirection fake_dir (const data::handle::port& port)
          {
            if (port.is_tunnel())
            {
              boost::optional<pnet::type::value::value_type> const tunnel_direction
                ( port.get().properties().get
                    ({"fhg", "pnete", "tunnel", "direction"})
                );

              return !tunnel_direction ? we::type::PORT_IN
                : boost::get<std::string> (*tunnel_direction) == "out"
                  ? we::type::PORT_OUT
                : boost::get<std::string> (*tunnel_direction) == "in"
                  ? we::type::PORT_IN
                : throw std::runtime_error
                  ("bad fhg.pnete.tunnel.direction (neither 'in' nor 'out')");
            }
            else
            {
              return port.get().direction();
            }
          }
        }

        QPainterPath pending_connection::shape() const
        {
          QList<QPointF> points;

          //! \todo automatic orthogonal lines
          if (const port_item* fix = qobject_cast<const port_item*> (_fixed_end))
          {
            const we::type::PortDirection dir (fake_dir (fix->handle()));
            if (dir == we::type::PORT_IN)
            {
              points.push_back (_open_end);
              points.push_back (_fixed_end->scenePos());
            }
            else if (dir == we::type::PORT_OUT)
            {
              points.push_back (_fixed_end->scenePos());
              points.push_back (_open_end);
            }
            else if (dir == we::type::PORT_TUNNEL)
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
