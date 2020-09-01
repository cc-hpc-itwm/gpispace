#include <drts/Forest.hpp>
#include <drts/Resource.hpp>
#include <drts/ResourceGraph.hpp>

#include <memory>

namespace gspc
{
  using Resources = UniqueForest<Resource>;
  ResourceGraph::ResourceGraph()
    : _implementation (new Resources())
  {}

  ResourceGraph::~ResourceGraph()
  { delete static_cast<Resources*>(_implementation); }

  std::uint64_t ResourceGraph::insert
    ( std::string const& resource
    , std::unordered_set<std::uint64_t> const& children
    )
  {
    return static_cast<Resources*>(_implementation)
      ->insert (resource, children);
  }

  std::uint64_t  ResourceGraph::insert
    ( std::string const& resource
    , std::unordered_set<std::uint64_t> const& children
    , std::uint64_t const& proxy
    )
  {
    return static_cast<Resources*>(_implementation)
      ->insert (resource, children, proxy);
  }
}
