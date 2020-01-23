#include <boost/serialization/optional.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/vector.hpp>

namespace gspc
{
  namespace rtm
  {
    template<typename Archive>
      void LoadInput::serialize (Archive& ar, unsigned int)
    {
      ar & parameter;
      ar & shot;
    }

    template<typename Archive>
      void LoadOutput::serialize (Archive& ar, unsigned int)
    {
      ar & shot;
    }

    template<typename Archive>
      void ProcessInput::serialize (Archive& ar, unsigned int)
    {
      ar & parameter;
      ar & shot;
    }

    template<typename Archive>
      void ProcessOutput::serialize (Archive& ar, unsigned int)
    {
      ar & result;
    }

    template<typename Archive>
      void ReduceInput::serialize (Archive& ar, unsigned int)
    {
      ar & parameter;
      ar & lhs;
      ar & rhs;
    }

    template<typename Archive>
      void ReduceOutput::serialize (Archive& ar, unsigned int)
    {
      ar & result;
    }

    template<typename Archive>
      void StoreInput::serialize (Archive& ar, unsigned int)
    {
      ar & parameter;
      ar & result;
    }

    template<typename Archive>
      void StoreOutput::serialize (Archive& ar, unsigned int)
    {
      ar & result;
    }
  }
}
