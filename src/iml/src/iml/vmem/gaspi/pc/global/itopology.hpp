// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/SegmentDescription.hpp>
#include <iml/SegmentHandle.hpp>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      class itopology_t
      {
      public:
        virtual ~itopology_t () = default;

        itopology_t() = default;
        itopology_t (itopology_t const&) = delete;
        itopology_t& operator= (itopology_t const&) = delete;
        itopology_t (itopology_t&&) = delete;
        itopology_t& operator= (itopology_t&&) = delete;

        // initiate a global alloc
        virtual void alloc ( iml::SegmentHandle segment
                           , iml::AllocationHandle
                           , iml::MemoryOffset
                           , iml::MemorySize size
                           , iml::MemorySize local_size
                           ) = 0;

        virtual void free (iml::AllocationHandle) = 0;

        virtual void add_memory ( iml::SegmentHandle seg_id
                                , iml::SegmentDescription const& description
                                , unsigned long total_size
                                ) = 0;

        virtual void del_memory (iml::SegmentHandle seg_id) = 0;
      };
    }
  }
}
