// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/MemoryLocation.hpp>

#include <util-generic/testing/random/impl.hpp>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        template<> struct random_impl<iml::MemoryLocation>
        {
          iml::MemoryLocation operator()() const;
        };
      }
    }
  }
}
