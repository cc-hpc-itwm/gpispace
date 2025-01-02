// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/testing/random/SegmentHandle.hpp>

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
        iml::SegmentHandle random_impl<iml::SegmentHandle>::operator()() const
        {
          return iml::SegmentHandle (random<std::uint64_t>{}());
        }
      }
    }
  }
}
