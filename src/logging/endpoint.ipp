#include <boost/serialization/optional.hpp>

namespace fhg
{
  namespace logging
  {
    template<typename Archive>
      void serialize (Archive& ar, endpoint& ep, unsigned int const)
    {
      ar & ep.as_tcp;
      ar & ep.as_socket;
    }
  }
}
