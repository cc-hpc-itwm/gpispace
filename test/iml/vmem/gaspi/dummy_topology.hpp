// Copyright (C) 2012,2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/SegmentDescription.hpp>
#include <gspc/iml/SegmentHandle.hpp>
#include <gspc/iml/vmem/gaspi/pc/global/itopology.hpp>


  namespace gpi::tests
  {
    class dummy_topology : public gpi::pc::global::itopology_t
    {
    public:
      void alloc ( gspc::iml::SegmentHandle /* segment */
                         , gspc::iml::AllocationHandle /* handle */
                , gspc::iml::MemoryOffset /* offset */
                , gspc::iml::MemorySize /* size */
                , gspc::iml::MemorySize /* local_size */
                ) override
      {}

      void free (gspc::iml::AllocationHandle) override
      {}

      void add_memory ( gspc::iml::SegmentHandle
                              , gspc::iml::SegmentDescription const&
                              , unsigned long
                     ) override
      {}

      void del_memory (gspc::iml::SegmentHandle) override
      {}
    };
  }
