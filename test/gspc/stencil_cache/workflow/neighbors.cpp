#include <gspc/stencil_cache/callback.hpp>
#include <test/gspc/stencil_cache/workflow/Neighborhood.hpp>

using N = test::gspc::stencil_cache::workflow::Neighborhood;

GSPC_STENCIL_CACHE_CALLBACK (void*, init) (std::vector<char> const& data)
{
  return new N (data);
}
GSPC_STENCIL_CACHE_CALLBACK (void, destroy) (void* neighborhood)
{
  delete static_cast<N*> (neighborhood);
}

GSPC_STENCIL_CACHE_CALLBACK (void, neighbors)
  ( void* neighborhood
  , gspc::stencil_cache::Stencil coordinate
  , std::list<gspc::stencil_cache::Coordinate>& neighbors
  )
{
  static_cast<N*> (neighborhood)->operator() (coordinate, neighbors);
}
