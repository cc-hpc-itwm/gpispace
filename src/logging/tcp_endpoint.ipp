#include <boost/serialization/utility.hpp>

namespace fhg
{
  namespace logging
  {
    template<typename Archive>
      void serialize (Archive& ar, tcp_endpoint& ep, unsigned int const)
    {
      ar & ep.host;
      ar & ep.port;
    }
  }
}
