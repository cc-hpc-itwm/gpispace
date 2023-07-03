// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
