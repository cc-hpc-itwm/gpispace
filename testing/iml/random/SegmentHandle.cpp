// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/iml/random/SegmentHandle.hpp>

#include <gspc/testing/random.hpp>

#include <cstdint>




      namespace gspc::testing::detail
      {
        gspc::iml::SegmentHandle random_impl<gspc::iml::SegmentHandle>::operator()() const
        {
          return gspc::iml::SegmentHandle (random<std::uint64_t>{}());
        }
      }
