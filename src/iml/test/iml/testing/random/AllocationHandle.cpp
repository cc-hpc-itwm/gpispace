// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/testing/random/AllocationHandle.hpp>

#include <util-generic/testing/random.hpp>

#include <cstdint>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        iml::AllocationHandle
          random_impl<iml::AllocationHandle>::operator()() const
        {
          return iml::AllocationHandle (random<std::uint64_t>{}());
        }
      }
    }
  }
}
