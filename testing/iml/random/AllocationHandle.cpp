// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/iml/random/AllocationHandle.hpp>

#include <gspc/testing/random.hpp>

#include <cstdint>




      namespace gspc::testing::detail
      {
        gspc::iml::AllocationHandle
          random_impl<gspc::iml::AllocationHandle>::operator()() const
        {
          return gspc::iml::AllocationHandle (random<std::uint64_t>{}());
        }
      }
