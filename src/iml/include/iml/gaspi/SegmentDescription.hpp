// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/detail/dllexport.hpp>

#include <cstddef>

namespace iml
{
  namespace gaspi
  {
    //! A GASPI backed segment using aggregated memory from the
    //! participating nodes.
    struct IML_DLLEXPORT SegmentDescription
    {
      //! Create a description for a GASPI segment. The transfers
      //! to/from the segment will be done using \a
      //! communication_buffer_count_ buffers of size \a
      //! communication_buffer_size_ on every node.
      SegmentDescription
        ( std::size_t communication_buffer_size_ = 4 * (1 << 20)
        , std::size_t communication_buffer_count_ = 8
        );

      //! Serialize using Boost.Serialization.
      template<typename BoostArchive>
        void serialize (BoostArchive&, unsigned int);

      std::size_t communication_buffer_size;
      std::size_t communication_buffer_count;
    };
  }
}

#include <iml/gaspi/SegmentDescription.ipp>
