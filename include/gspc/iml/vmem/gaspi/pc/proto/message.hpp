// Copyright (C) 2011,2015,2018,2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <variant>

// serialization
#include <boost/mpl/vector.hpp>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <gspc/util/serialization/std/variant.hpp>

#include <gspc/iml/vmem/gaspi/pc/proto/error.hpp>
#include <gspc/iml/vmem/gaspi/pc/proto/memory.hpp>
#include <gspc/iml/vmem/gaspi/pc/proto/segment.hpp>

#include <unordered_set>



    namespace gpi::pc::proto
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
        gspc::iml::SegmentHandle segment;
        template<typename Archive>
          void serialize (Archive& ar, unsigned int)
        {
          ar & segment;
        }
      };

      using message_t = std::variant< error::error_t
                                    , gpi::pc::proto::memory::message_t
                                    , gpi::pc::proto::segment::message_t
                                    , existing_segments
                                    , existing_allocations
                                    , std::unordered_set<gspc::iml::SegmentHandle>
                                    , std::unordered_set<gspc::iml::AllocationHandle>
                                    >;
    }
