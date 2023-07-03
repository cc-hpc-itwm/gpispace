// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/AllocationHandle.hpp>
#include <iml/Client.hpp>
#include <iml/MemoryLocation.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemoryRegion.hpp>
#include <iml/MemorySize.hpp>
#include <iml/SegmentDescription.hpp>
#include <iml/SegmentHandle.hpp>
#include <iml/detail/dllexport.hpp>

#include <boost/optional.hpp>

namespace iml
{
  //! A scoped wrapper for a global segment and allocation in it, for
  //! convenience over the \c Client API.
  class IML_DLLEXPORT SegmentAndAllocation
  {
  public:
    //! Create a global segment using \a client with the type and
    //! parameters defined by \a description sized \a total_size
    //! bytes, and create an allocation inside it that fully fills it.
    //! \note The given \c Client reference has to outlive this object.
    //! \see Client::create_segment(), Client::create_allocation()
    SegmentAndAllocation ( Client& client
                         , SegmentDescription description
                         , MemorySize total_size
                         );

    //! Create a global segment using \a client with the type and
    //! parameters defined by \a description sized \a total_size
    //! bytes, and create an allocation inside it that fully fills it,
    //! and then copy \a total_size bytes of \a data into it.
    //! \note The given \c Client reference has to outlive this object.
    //! \note A shared memory segment with sufficient size is
    //! temporarily created and does not need to exist yet.
    //! \see Client::create_segment(), Client::create_allocation()
    SegmentAndAllocation ( Client& client
                         , SegmentDescription description
                         , MemorySize total_size
                         , char const* data
                         );

    //! Reference this allocation at the given \a offset.
    MemoryLocation memory_location (MemoryOffset offset = 0) const;
    //! Reference this allocation at the given \a offset and \a size.
    MemoryRegion memory_region (MemoryOffset offset, MemorySize size) const;

    //! Retrieve the size requested for the allocation.
    MemorySize size() const;

    SegmentAndAllocation() = delete;
    SegmentAndAllocation (SegmentAndAllocation const&) = delete;
    SegmentAndAllocation (SegmentAndAllocation&&) noexcept;
    SegmentAndAllocation& operator= (SegmentAndAllocation const&) = delete;
    SegmentAndAllocation& operator= (SegmentAndAllocation&&) = delete;
    ~SegmentAndAllocation();

  private:
    Client& _client;
    MemorySize const _size;
    ::boost::optional<SegmentHandle> _segment;
    ::boost::optional<AllocationHandle> _allocation;
  };
}
