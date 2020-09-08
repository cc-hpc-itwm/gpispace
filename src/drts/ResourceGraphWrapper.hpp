#include <drts/Forest.hpp>
#include <drts/Resource.hpp>

namespace gspc
{
  using Resources = UniqueForest<Resource>;

  struct ResourceGraph::implementation
  {
    Resources _resources;
  };
}
