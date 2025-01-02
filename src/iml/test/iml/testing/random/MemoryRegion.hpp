// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/MemoryRegion.hpp>

#include <util-generic/testing/random/impl.hpp>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<> struct random_impl<iml::MemoryRegion>
        {
          iml::MemoryRegion operator()() const;
        };
      }
    }
  }
}
