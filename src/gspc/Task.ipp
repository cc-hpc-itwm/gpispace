#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <boost/serialization/unordered_map.hpp>

namespace gspc
{
  template<typename Archive>
    void Task::serialize (Archive& ar, unsigned int)
  {
    ar & id;
    ar & resource_class;
    ar & input;
    ar & so;
    ar & symbol;
  }
}
