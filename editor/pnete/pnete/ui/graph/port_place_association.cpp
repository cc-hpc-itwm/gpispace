// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/port_place_association.hpp>

#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/port.hpp>

#include <xml/parse/type/port.hpp>

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
        {
          //! \todo Getting direction should be encapsuled in handle.
          //! \todo Add handling of port direction getting changed!
          if (handle.get().direction() == we::type::PORT_OUT)
          {
            invert();
          }
        }

        const data::handle::port& port_place_association::handle() const
        {
          return _handle;
        }
      }
    }
  }
}
