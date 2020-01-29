#include <boost/serialization/vector.hpp>

namespace gspc
{
  namespace workflow_engine
  {
    template<typename Archive>
      void State::serialize (Archive& ar, unsigned int)
    {
      ar & engine_specific;
      ar & processing_state;
    }
  }
}
