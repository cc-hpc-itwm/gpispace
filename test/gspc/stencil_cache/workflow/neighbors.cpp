#include <gspc/stencil_cache/callback.hpp>
#include <test/gspc/stencil_cache/workflow/Neighborhood.hpp>

using N = test::gspc::stencil_cache::workflow::Neighborhood;

GSPC_STENCIL_CACHE_CALLBACK (std::shared_ptr<void>, init)
  (std::vector<char> const& data)
{
  return std::make_shared<N> (data);
}

GSPC_STENCIL_CACHE_CALLBACK
  (std::list<gspc::stencil_cache::Coordinate>, neighbors)
  ( void* neighborhood
  , gspc::stencil_cache::Stencil coordinate
  )
{
  return static_cast<N*> (neighborhood)->operator() (coordinate);
}
