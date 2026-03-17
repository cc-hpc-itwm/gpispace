// Copyright (C) 2018,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>



    namespace gspc::testing
    {
      //! Number of bits needed to represent the internal state of a
      //! given RandomNumberEngine
      template<typename RandomNumberEngine>
        constexpr std::size_t state_bits();
    }



#include <gspc/testing/random/state_bits.ipp>
