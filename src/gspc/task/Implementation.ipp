#include <util-generic/serialization/boost/filesystem/path.hpp>

namespace gspc
{
  namespace task
  {
    template<typename Archive>
      void Implementation::serialize (Archive& ar, unsigned int)
    {
      ar & so;
      ar & symbol;
    }
  }
}
