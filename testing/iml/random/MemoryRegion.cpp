// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/iml/random/MemoryRegion.hpp>

#include <gspc/iml/MemoryLocation.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/testing/iml/random/MemoryLocation.hpp>

#include <gspc/testing/random.hpp>




      namespace gspc::testing::detail
      {
        gspc::iml::MemoryRegion random_impl<gspc::iml::MemoryRegion>::operator()() const
        {
          return { random<gspc::iml::MemoryLocation>{}()
                 , random<gspc::iml::MemorySize>{}()
                 };
        }
      }
