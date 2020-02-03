#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/utility.hpp>

namespace gspc
{
  template<typename Archive>
    void Task::serialize (Archive& ar, unsigned int)
  {
    ar & input;
    ar & requirements;
  }
}
