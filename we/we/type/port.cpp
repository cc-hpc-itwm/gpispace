// mirko.rahn@itwm.fhg.de

#include <we/type/port.hpp>

#include <iostream>
#include <stdexcept>

namespace we
{
  namespace type
  {
    std::ostream& operator<< (std::ostream& s, const PortDirection& p)
    {
      switch (p)
        {
        case PORT_IN: return s << "port-in";
        case PORT_OUT: return s << "port-out";
        case PORT_IN_OUT: return s << "port-inout";
        case PORT_TUNNEL: return s << "port-tunnel";
        default:
          throw std::runtime_error ("STRANGE: unknown PortDirection");
        }
    }

    std::ostream& operator<< (std::ostream& os, const port_t& p)
    {
      os << "{port, "
         << p.direction()
         << ", "
         << p.name()
         << ", "
         << p.signature()
         << ", "
        ;

      if (p.associated_place() == petri_net::place_id_invalid())
        {
          os << "not associated";
        }
      else
        {
          os << "associated with place " << p.associated_place();
        }
      os << "}";

      return os;
    }
  }
}
