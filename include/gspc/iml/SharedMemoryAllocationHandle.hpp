// Copyright (C) 2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/detail/export.hpp>

namespace gspc::iml
{
  //! The handle of a local shared memory allocation.
  struct GSPC_EXPORT SharedMemoryAllocationHandle : public AllocationHandle
  {
    //! Create an allocation handle from a \a raw_handle.
    explicit SharedMemoryAllocationHandle (AllocationHandle raw_handle);

    //! \note For serialization only.
    SharedMemoryAllocationHandle() = default;

    //! Serialize using Boost.Serialization.
    template<typename BoostArchive>
      void serialize (BoostArchive& archive, unsigned int);
  };
}

#include <gspc/iml/SharedMemoryAllocationHandle.ipp>
