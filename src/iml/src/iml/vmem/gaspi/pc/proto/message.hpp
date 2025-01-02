// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
        void clear()
        {
          length = 0;
        }

        uint32_t     length {0};
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

      using message_impl = ::boost::variant< error::error_t
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
          ar & ::boost::serialization::base_object<message_impl> (*this);
        }
      };
    }
  }
}
