// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <we/type/port.hpp>

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
      }
    }
  }
}
