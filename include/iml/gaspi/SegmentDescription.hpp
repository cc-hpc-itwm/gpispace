// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
