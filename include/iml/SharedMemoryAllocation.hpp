// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
#include <iml/Client.hpp>
#include <iml/MemoryLocation.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemoryRegion.hpp>
#include <iml/MemorySize.hpp>
#include <iml/detail/dllexport.hpp>

namespace iml
{
  //! A scoped wrapper for a local shared memory allocation, for
  //! convenience over the \c Client API.
  class IML_DLLEXPORT SharedMemoryAllocation
  {
  public:
    //! Create a new local shared memory allocation using the given \a
    //! client that can be used to transfer data up to \a size bytes
    //! from global segments into the local process.
    //! \note The given \c Client reference has to outlive this object.
    //! \see Client::create_shm_segment_and_allocate()
    SharedMemoryAllocation (Client& client, MemorySize size);

    //! Reference this allocation at the given \a offset.
    MemoryLocation memory_location (MemoryOffset offset = 0) const;
    //! Reference this allocation at the given \a offset and \a size.
    MemoryRegion memory_region (MemoryOffset offset, MemorySize size) const;

    //! Retrieve the size requested for the allocation.
    MemorySize size() const;

    //! Retrieve a pointer into this shared memory allocation. The
    //! pointer can be read and written to freely, but interleaving
    //! access with transfers started by \c Client::async_memcpy() is
    //! unspecified behavior.
    //! \see Client::pointer()
    char* pointer() const;

    // \todo Also add memcpy wrappers?

    SharedMemoryAllocation() = delete;
    SharedMemoryAllocation (SharedMemoryAllocation const&) = delete;
    SharedMemoryAllocation (SharedMemoryAllocation&&);
    SharedMemoryAllocation& operator= (SharedMemoryAllocation const&) = delete;
    SharedMemoryAllocation& operator= (SharedMemoryAllocation&&) = delete;
    ~SharedMemoryAllocation();

  private:
    Client* _client;
    SharedMemoryAllocationHandle const _handle;
    MemorySize const _size;
  };
}
