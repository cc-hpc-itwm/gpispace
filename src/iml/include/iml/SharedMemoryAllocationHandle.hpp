// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/AllocationHandle.hpp>
#include <iml/detail/dllexport.hpp>

namespace iml
{
  //! The handle of a local shared memory allocation.
  struct IML_DLLEXPORT SharedMemoryAllocationHandle : public AllocationHandle
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

#include <iml/SharedMemoryAllocationHandle.ipp>
