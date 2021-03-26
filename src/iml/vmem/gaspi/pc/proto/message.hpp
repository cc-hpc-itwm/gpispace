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

#include <boost/variant.hpp>

// serialization
#include <boost/mpl/vector.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/variant.hpp>

#include <iml/vmem/gaspi/pc/proto/error.hpp>
#include <iml/vmem/gaspi/pc/proto/memory.hpp>
#include <iml/vmem/gaspi/pc/proto/segment.hpp>

#include <unordered_set>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      struct header_t
      {
        header_t ()
          : length (0)
        {}

        void clear()
        {
          length = 0;
        }

        uint32_t     length;
      };

      struct existing_segments
      {
        template<typename Archive>
          void serialize (Archive&, unsigned int)
        {
        }
      };
      struct existing_allocations
      {
        iml::SegmentHandle segment;
        template<typename Archive>
          void serialize (Archive& ar, unsigned int)
        {
          ar & segment;
        }
      };

      using message_impl = boost::variant< error::error_t
                                         , gpi::pc::proto::memory::message_t
                                         , gpi::pc::proto::segment::message_t
                                         , existing_segments
                                         , existing_allocations
                                         , std::unordered_set<iml::SegmentHandle>
                                         , std::unordered_set<iml::AllocationHandle>
                                         >;
      struct message_t : message_impl
      {
        using message_impl::message_impl;

        template<typename Archive>
          void serialize (Archive& ar, unsigned int)
        {
          ar & boost::serialization::base_object<message_impl> (*this);
        }
      };
    }
  }
}
