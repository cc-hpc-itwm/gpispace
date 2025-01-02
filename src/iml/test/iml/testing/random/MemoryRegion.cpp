// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/testing/random/MemoryRegion.hpp>

#include <iml/MemoryLocation.hpp>
#include <iml/MemorySize.hpp>
#include <iml/testing/random/MemoryLocation.hpp>

#include <util-generic/testing/random.hpp>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        iml::MemoryRegion random_impl<iml::MemoryRegion>::operator()() const
        {
          return { random<iml::MemoryLocation>{}()
                 , random<iml::MemorySize>{}()
                 };
        }
      }
    }
  }
}
