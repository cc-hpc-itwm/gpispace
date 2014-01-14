// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <we/type/port.hpp>

#include <we/type/signature/show.hpp>

#include <iostream>
#include <stdexcept>

namespace we
{
  namespace type
  {
    std::string enum_to_string (const PortDirection& dir)
    {
      switch (dir)
      {
      case PORT_IN: return "in";
      case PORT_OUT: return "out";
      case PORT_TUNNEL: return "tunnel";
      default:
        throw std::runtime_error ("STRANGE: unknown PortDirection");
      }
    }

    std::ostream& operator<< (std::ostream& s, const PortDirection& p)
    {
      return s << "port-" << enum_to_string (p);
    }

    std::ostream& operator<< (std::ostream& os, const port_t& p)
    {
      os << "{port, "
         << p.direction()
         << ", "
         << p.name()
         << ", "
         << pnet::type::signature::show (p.signature())
         << ", "
        ;

      if (p.associated_place() == we::place_id_invalid())
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
