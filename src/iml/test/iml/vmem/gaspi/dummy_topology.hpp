// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/SegmentDescription.hpp>
#include <iml/SegmentHandle.hpp>
#include <iml/vmem/gaspi/pc/global/itopology.hpp>

namespace gpi
{
  namespace tests
  {
    class dummy_topology : public gpi::pc::global::itopology_t
    {
    public:
      void alloc ( iml::SegmentHandle /* segment */
                         , iml::AllocationHandle /* handle */
                , iml::MemoryOffset /* offset */
                , iml::MemorySize /* size */
                , iml::MemorySize /* local_size */
                ) override
      {}

      void free (iml::AllocationHandle) override
      {}

      void add_memory ( iml::SegmentHandle
                              , iml::SegmentDescription const&
                              , unsigned long
                     ) override
      {}

      void del_memory (iml::SegmentHandle) override
      {}
    };
  }
}
