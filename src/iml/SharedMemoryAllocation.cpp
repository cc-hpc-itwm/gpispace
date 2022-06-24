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

#include <iml/SharedMemoryAllocation.hpp>

namespace iml
{
  SharedMemoryAllocation::SharedMemoryAllocation
      (Client& client, MemorySize size)
    : _client (&client)
    , _handle (_client->create_shm_segment_and_allocate (size))
    , _size (size)
  {}

  SharedMemoryAllocation::SharedMemoryAllocation
      ( SharedMemoryAllocation&& other
      ) noexcept
        : _client (other._client)
        , _handle (other._handle)
        , _size (other._size)
  {
    other._client = nullptr;
  }
  SharedMemoryAllocation::~SharedMemoryAllocation()
  {
    if (_client)
    {
      _client->free_and_delete_shm_segment (_handle);
    }
  }

  MemoryLocation SharedMemoryAllocation::memory_location
    (MemoryOffset offset) const
  {
    return {_handle, offset};
  }
  MemoryRegion SharedMemoryAllocation::memory_region
    (MemoryOffset offset, MemorySize size) const
  {
    return {memory_location (offset), size};
  }

  char* SharedMemoryAllocation::pointer() const
  {
    return _client->pointer (_handle);
  }

  MemorySize SharedMemoryAllocation::size() const
  {
    return _size;
  }
}
