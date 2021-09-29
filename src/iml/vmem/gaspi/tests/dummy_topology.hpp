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
      virtual void alloc ( iml::SegmentHandle /* segment */
                         , iml::AllocationHandle /* handle */
                , iml::MemoryOffset /* offset */
                , iml::MemorySize /* size */
                , iml::MemorySize /* local_size */
                ) override
      {}

      virtual void free (iml::AllocationHandle) override
      {}

      virtual void add_memory ( iml::SegmentHandle
                              , iml::SegmentDescription const&
                              , unsigned long
                     ) override
      {}

      virtual void del_memory (iml::SegmentHandle) override
      {}
    };
  }
}
