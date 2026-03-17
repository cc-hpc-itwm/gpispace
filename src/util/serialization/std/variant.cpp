#include <fmt/core.h>
#include <gspc/util/serialization/std/variant.hpp>

namespace gspc::util
{
  auto variant_index_out_of_bound
    ( std::size_t i
    , std::size_t N
    ) -> std::logic_error
  {
    return std::logic_error
      { fmt::format ("Variant index out of bounds: ! ({} < {})", i, N)
      };
  }
}
