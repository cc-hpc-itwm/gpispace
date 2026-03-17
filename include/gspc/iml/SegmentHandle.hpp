// Copyright (C) 2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <cstdint>
#include <functional>
#include <istream>
#include <ostream>
#include <string>

namespace gspc::iml
{
  //! The handle of a memory segment, obtained via \c Client when
  //! creating a segment.
  class GSPC_EXPORT SegmentHandle
  {
  public:
    //! Create a segment handle from a \a serialized representation
    //! produced with \c to_string().
    //! \see to_string()
    SegmentHandle (std::string const& serialized);
    //! Serialize the handle into a string.
    std::string to_string() const;

    GSPC_EXPORT friend std::ostream& operator<<
      (std::ostream&, SegmentHandle const&);
    GSPC_EXPORT friend std::istream& operator>>
      (std::istream&, SegmentHandle&);
    GSPC_EXPORT friend bool operator==
      (SegmentHandle const&, SegmentHandle const&);
    GSPC_EXPORT friend bool operator<
      (SegmentHandle const&, SegmentHandle const&);
    friend GSPC_EXPORT std::hash<SegmentHandle>;

    //! Create a segment handle from a \a raw_handle.
    explicit SegmentHandle (std::uint64_t raw_handle);

    //! \note For serialization only.
    SegmentHandle() = default;

    //! Serialize using Boost.Serialization.
    template<typename BoostArchive>
      void serialize (BoostArchive& archive, unsigned int);

  private:
    std::uint64_t handle;
  };
}

namespace std
{
  template<> struct hash<gspc::iml::SegmentHandle>
  {
    size_t operator() (gspc::iml::SegmentHandle const&) const;
  };
}

#include <gspc/iml/SegmentHandle.ipp>
