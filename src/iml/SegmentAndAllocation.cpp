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

  SegmentAndAllocation::SegmentAndAllocation (SegmentAndAllocation&& other)
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
