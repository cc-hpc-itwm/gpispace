// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      //! Number of bits needed to represent the internal state of a
      //! given RandomNumberEngine
      template<typename RandomNumberEngine>
        constexpr std::size_t state_bits();
    }
  }
}

#include <util-generic/testing/random/state_bits.ipp>
