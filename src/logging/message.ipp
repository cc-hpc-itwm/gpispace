#include <util-generic/serialization/std/chrono.hpp>

namespace fhg
{
  namespace logging
  {
    template<typename Archive>
      void message::serialize (Archive& ar, unsigned int)
    {
      ar & _content;
      ar & _category;
      ar & _timestamp;
      ar & _hostname;
      ar & _process_id;
      ar & _thread_id;
    }
  }
}
