// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/iml/random/MemoryLocation.hpp>

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/testing/iml/random/AllocationHandle.hpp>

#include <gspc/testing/random.hpp>




      namespace gspc::testing::detail
      {
        gspc::iml::MemoryLocation
          random_impl<gspc::iml::MemoryLocation>::operator()() const
        {
          return { random<gspc::iml::AllocationHandle>{}()
                 , random<gspc::iml::MemoryOffset>{}()
                 };
        }
      }
