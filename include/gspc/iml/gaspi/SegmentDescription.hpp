// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <cstddef>


  namespace gspc::iml::gaspi
  {
    //! A GASPI backed segment using aggregated memory from the
    //! participating nodes.
    struct GSPC_EXPORT SegmentDescription
    {
      //! Create a description for a GASPI segment. The transfers
      //! to/from the segment will be done using \a
      //! communication_buffer_count_ buffers of size \a
      //! communication_buffer_size_ on every node.
      SegmentDescription
        ( std::size_t communication_buffer_size_ = 4UL * (1UL << 20UL)
        , std::size_t communication_buffer_count_ = 8UL
        );

      //! Serialize using Boost.Serialization.
      template<typename BoostArchive>
        void serialize (BoostArchive&, unsigned int);

      std::size_t communication_buffer_size;
      std::size_t communication_buffer_count;
    };
  }


#include <gspc/iml/gaspi/SegmentDescription.ipp>
