#include <drts/Forest.hpp>
#include <drts/ResourceGraph.hpp>
#include <drts/ResourceGraphWrapper.hpp>

#include <memory>

namespace gspc
{
  ResourceGraph::ResourceGraph()
    : _ (std::make_unique<implementation>())
  {}

  ResourceGraph::~ResourceGraph() = default;

  std::uint64_t ResourceGraph::insert
    ( Resource const& resource
    , std::unordered_set<std::uint64_t> const& children
    )
  {
    return _->_resources.insert (resource, children);
  }

  std::uint64_t  ResourceGraph::insert
    ( Resource const& resource
    , std::unordered_set<std::uint64_t> const& children
    , std::uint64_t const& proxy
    )
  {
    return _->_resources.insert (resource, children, proxy);
  }
}
