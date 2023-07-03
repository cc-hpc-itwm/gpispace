// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/SegmentAndAllocation.hpp>

#include <iml/SharedMemoryAllocation.hpp>

#include <boost/format.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace iml
{
  SegmentAndAllocation::SegmentAndAllocation
      ( Client& client
      , SegmentDescription description
      , MemorySize size
      )
    : _client (client)
    , _size (size)
    , _segment (_client.create_segment (description, _size))
    , _allocation (_client.create_allocation (*_segment, _size))
  {}

  SegmentAndAllocation::SegmentAndAllocation
      ( Client& client
      , SegmentDescription description
      , MemorySize size
      , char const* const data
      )
    : SegmentAndAllocation (client, description, size)
  {
    SharedMemoryAllocation const buffer (client, size);

    char* const content (buffer.pointer());
    std::copy (data, data + size, content);

    client.memcpy (memory_location(), buffer.memory_location(), size);
  }

  SegmentAndAllocation::~SegmentAndAllocation()
  {
    if (_allocation)
    {
      _client.delete_allocation (*_allocation);
    }
    if (_segment)
    {
       _client.delete_segment (*_segment);
    }
  }

  SegmentAndAllocation::SegmentAndAllocation
    ( SegmentAndAllocation&& other
    ) noexcept
      : _client (other._client)
      , _size (std::move (other._size))
      , _segment (std::move (other._segment))
      , _allocation (std::move (other._allocation))
  {
    other._segment.reset();
    other._allocation.reset();
  }

  MemoryLocation SegmentAndAllocation::memory_location
    (MemoryOffset offset) const
  {
    return {*_allocation, offset};
  }

  MemoryRegion SegmentAndAllocation::memory_region
    (MemoryOffset offset, MemorySize size) const
  {
    if ((offset + size) > _size)
    {
      throw std::logic_error
        ( (::boost::format ("memory region [%1%, %2%) is outside of allocation")
          % offset % (offset + size)
          ).str()
        );
    }

    return {memory_location (offset), size};
  }

  MemorySize SegmentAndAllocation::size() const
  {
    return _size;
  }
}
