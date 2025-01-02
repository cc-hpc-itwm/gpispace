// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/AllocationHandle.hpp>
#include <iml/SegmentHandle.hpp>

#include <atomic>
#include <cstddef>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class handle_generator_t
      {
      public:
        //! Initialize a handle generator which will generate unique
        //! handles with the global part \a identifier.
        explicit handle_generator_t (std::size_t identifier);

        //! Generate the next handle. Thread-safe.
        iml::AllocationHandle next_allocation();

        //! Generate the next handle. Thread-safe.
        iml::SegmentHandle next_segment();

      private:
        std::size_t m_node_identifier;
        std::atomic<std::size_t> m_counter;
      };
    }
  }
}
