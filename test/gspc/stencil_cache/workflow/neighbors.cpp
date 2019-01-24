#include <gspc/stencil_cache/callback.hpp>
#include <test/gspc/stencil_cache/workflow/size.hpp>

#include <algorithm>

GSPC_STENCIL_CACHE_CALLBACK (void, neighbors)
  ( gspc::stencil_cache::Stencil coordinate
  , std::list<gspc::stencil_cache::Coordinate>& neighbors
  )
{
  auto const X (test::gspc::stencil_cache::workflow::size::X());
  auto const Y (test::gspc::stencil_cache::workflow::size::Y());
  auto const R (test::gspc::stencil_cache::workflow::size::R());

  auto const px (coordinate / Y);
  auto const py (coordinate % Y);

  neighbors.clear();

  for (long x {std::max (0L, px - R + 1L)}; x < std::min (X, px + R); ++x)
  for (long y {std::max (0L, py - R + 1L)}; y < std::min (Y, py + R); ++y)
  {
    neighbors.emplace_back (y + Y * x);
  }
}
