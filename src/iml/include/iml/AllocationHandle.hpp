// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/detail/dllexport.hpp>

#include <cstdint>
#include <functional>
#include <istream>
#include <ostream>
#include <string>

namespace iml
{
  //! The handle of an allocation in a memory segment, obtained via
  //! \c Client::create_allocation() or indirectly via the
  //! \c SharedMemoryAllocationHandle returned by
  //! \c Client::create_shm_segment_and_allocate().
  class IML_DLLEXPORT AllocationHandle
  {
  public:
    //! Create an allocation handle from a \a serialized
    //! representation produced with \c to_string().
    //! \see to_string()
    AllocationHandle (std::string const& serialized);
    //! Serialize the handle into a string.
    std::string to_string() const;

    IML_DLLEXPORT friend std::ostream& operator<<
      (std::ostream&, AllocationHandle const&);
    IML_DLLEXPORT friend std::istream& operator>>
      (std::istream&, AllocationHandle&);
    IML_DLLEXPORT friend bool operator==
      (AllocationHandle const&, AllocationHandle const&);
    IML_DLLEXPORT friend bool operator<
      (AllocationHandle const&, AllocationHandle const&);
    friend IML_DLLEXPORT std::hash<AllocationHandle>;

    //! Create an allocation handle from a \a raw_handle.
    explicit AllocationHandle (std::uint64_t raw_handle);

    //! \note For serialization only.
    AllocationHandle() = default;

    //! Serialize using Boost.Serialization.
    template<typename BoostArchive>
      void serialize (BoostArchive& archive, unsigned int);

  private:
    std::uint64_t handle;
  };
}

namespace std
{
  template<> struct hash<iml::AllocationHandle>
  {
    size_t operator() (iml::AllocationHandle const&) const;
  };
}

#include <iml/AllocationHandle.ipp>
