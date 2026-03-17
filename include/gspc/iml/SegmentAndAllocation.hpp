// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/Client.hpp>
#include <gspc/iml/MemoryLocation.hpp>
#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemoryRegion.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/SegmentDescription.hpp>
#include <gspc/iml/SegmentHandle.hpp>
#include <gspc/detail/export.hpp>

#include <optional>

namespace gspc::iml
{
  //! A scoped wrapper for a global segment and allocation in it, for
  //! convenience over the \c Client API.
  class GSPC_EXPORT SegmentAndAllocation
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
    std::optional<SegmentHandle> _segment;
    std::optional<AllocationHandle> _allocation;
  };
}
