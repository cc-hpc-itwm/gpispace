// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/testing/random/MemoryLocation.hpp>

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/testing/random/AllocationHandle.hpp>

#include <util-generic/testing/random.hpp>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        iml::MemoryLocation
          random_impl<iml::MemoryLocation>::operator()() const
        {
          return { random<iml::AllocationHandle>{}()
                 , random<iml::MemoryOffset>{}()
                 };
        }
      }
    }
  }
}
