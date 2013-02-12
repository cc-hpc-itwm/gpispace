// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/port_place_association.hpp>

#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/style/association.hpp>

#include <xml/parse/type/port.hpp>

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
          boost::optional<Qt::PenStyle> pen_style (const base_item* item)
          {
            return Qt::DotLine;
          }
        }

        port_place_association::port_place_association
          (port_item* port, place_item* place, const data::handle::port& handle)
            : association (port, place)
            , _handle (handle)
        {
          //! \todo Add handling of port direction getting changed!
          if (handle.is_output())
          {
            invert();
          }

          _style.push<Qt::PenStyle> ("border_style", mode::NORMAL, pen_style);

          handle.connect_to_change_mgr
            ( this
            , "place_association_set"
            , "data::handle::port, boost::optional<std::string>"
            );
        }

        const data::handle::port& port_place_association::handle() const
        {
          return _handle;
        }

        QPainterPath port_place_association::shape() const
        {
          if (handle().is_tunnel())
          {
            return style::association::shape_no_cap (all_points());
          }

          return association::shape();
        }

        void port_place_association::place_association_set
          ( const data::handle::port& changed_handle
          , const boost::optional<std::string>& place
          )
        {
          if (changed_handle == handle() && !place)
          {
            scene()->removeItem (this);
            deleteLater();
          }
        }
      }
    }
  }
}
