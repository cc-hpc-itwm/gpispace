#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <boost/serialization/list.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/utility.hpp>

namespace gspc
{
  template<typename Archive>
    void Task::serialize (Archive& ar, unsigned int)
  {
    ar & id;
    ar & input;
    ar & requirements;
  }
}
