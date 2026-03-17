// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/SegmentHandle.hpp>

#include <gspc/testing/random/impl.hpp>




      namespace gspc::testing::detail
      {
        template<> struct random_impl<gspc::iml::SegmentHandle>
        {
          gspc::iml::SegmentHandle operator()() const;
        };
      }
