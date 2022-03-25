// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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
