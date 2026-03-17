// Copyright (C) 2012,2014-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/SegmentDescription.hpp>
#include <gspc/iml/SegmentHandle.hpp>



    namespace gpi::pc::global
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
        virtual void alloc ( gspc::iml::SegmentHandle segment
                           , gspc::iml::AllocationHandle
                           , gspc::iml::MemoryOffset
                           , gspc::iml::MemorySize size
                           , gspc::iml::MemorySize local_size
                           ) = 0;

        virtual void free (gspc::iml::AllocationHandle) = 0;

        virtual void add_memory ( gspc::iml::SegmentHandle seg_id
                                , gspc::iml::SegmentDescription const& description
                                , unsigned long total_size
                                ) = 0;

        virtual void del_memory (gspc::iml::SegmentHandle seg_id) = 0;
      };
    }
