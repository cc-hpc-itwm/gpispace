// Copyright (C) 2023 Fraunhofer ITWM
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
  //! The handle of a memory segment, obtained via \c Client when
  //! creating a segment.
  class IML_DLLEXPORT SegmentHandle
  {
  public:
    //! Create a segment handle from a \a serialized representation
    //! produced with \c to_string().
    //! \see to_string()
    SegmentHandle (std::string const& serialized);
    //! Serialize the handle into a string.
    std::string to_string() const;

    IML_DLLEXPORT friend std::ostream& operator<<
      (std::ostream&, SegmentHandle const&);
    IML_DLLEXPORT friend std::istream& operator>>
      (std::istream&, SegmentHandle&);
    IML_DLLEXPORT friend bool operator==
      (SegmentHandle const&, SegmentHandle const&);
    IML_DLLEXPORT friend bool operator<
      (SegmentHandle const&, SegmentHandle const&);
    friend IML_DLLEXPORT std::hash<SegmentHandle>;

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
  template<> struct hash<iml::SegmentHandle>
  {
    size_t operator() (iml::SegmentHandle const&) const;
  };
}

#include <iml/SegmentHandle.ipp>
