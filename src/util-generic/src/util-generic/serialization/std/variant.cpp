// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fmt/core.h>
#include <util-generic/serialization/std/variant.hpp>

namespace fhg::util
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
