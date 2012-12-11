// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/port_place_association.hpp>

#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/port.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        port_place_association::port_place_association
          (port_item* port, place_item* place, const data::handle::port& handle)
            : association (port, place)
            , _handle (handle)
        { }

        const data::handle::port& port_place_association::handle() const
        {
          return _handle;
        }

        QPainterPath port_place_association::shape () const
        {
          //! \todo This should be visually different and is currently
          //! not regarding the port's direction.
          return association::shape();
        }

        void port_place_association::paint
          (QPainter* painter, const QStyleOptionGraphicsItem* opt, QWidget* wid)
        {
          association::paint (painter, opt, wid);
        }
      }
    }
  }
}
