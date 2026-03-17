// Copyright (C) 2011,2014-2015,2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/SegmentHandle.hpp>

#include <atomic>
#include <cstddef>



    namespace gpi::pc::memory
    {
      class handle_generator_t
      {
      public:
        //! Initialize a handle generator which will generate unique
        //! handles with the global part \a identifier.
        explicit handle_generator_t (std::size_t identifier);

        //! Generate the next handle. Thread-safe.
        gspc::iml::AllocationHandle next_allocation();

        //! Generate the next handle. Thread-safe.
        gspc::iml::SegmentHandle next_segment();

      private:
        std::size_t m_node_identifier;
        std::atomic<std::size_t> m_counter;
      };
    }
