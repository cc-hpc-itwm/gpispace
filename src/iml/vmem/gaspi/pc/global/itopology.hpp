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

        // initiate a global alloc
        virtual void alloc ( const iml::SegmentHandle segment
                           , const iml::AllocationHandle
                           , const iml::MemoryOffset
                           , const iml::MemorySize size
                           , const iml::MemorySize local_size
                           ) = 0;

        virtual void free (const iml::AllocationHandle) = 0;

        virtual void add_memory ( const iml::SegmentHandle seg_id
                                , iml::SegmentDescription const& description
                                , unsigned long total_size
                                ) = 0;

        virtual void del_memory (const iml::SegmentHandle seg_id) = 0;
      };
    }
  }
}
