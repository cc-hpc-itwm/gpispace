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

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <drts/rifd_entry_points.fwd.hpp>
#include <drts/pimpl.hpp>

#include <boost/filesystem/path.hpp>

namespace gspc
{
  class GSPC_DLLEXPORT rifd_entry_points
  {
  public:
    rifd_entry_points (boost::filesystem::path const&);

    rifd_entry_points (rifd_entry_points const&);

    void write_to_file (boost::filesystem::path const&);

  private:
    friend class rifds;
    friend class scoped_rifds;
    friend class scoped_runtime_system;

    PIMPL (rifd_entry_points);

    rifd_entry_points (implementation*);
  };

  class GSPC_DLLEXPORT rifd_entry_point
  {
  private:
    PIMPL (rifd_entry_point);

  public:
    rifd_entry_point (implementation*);
    rifd_entry_point (rifd_entry_point&&);

    std::string const& hostname() const;

  private:
    friend class scoped_rifd;
    friend class scoped_runtime_system;

    rifd_entry_point (rifd_entry_point const&) = delete;
    rifd_entry_point& operator= (rifd_entry_point&&) = delete;
    rifd_entry_point& operator= (rifd_entry_point const&) = delete;

    friend struct rifd_entry_point_hash;
    friend bool operator== (rifd_entry_point const&, rifd_entry_point const&);
  };

  struct GSPC_DLLEXPORT rifd_entry_point_hash
  {
    std::size_t operator() (rifd_entry_point const&) const;
  };
  GSPC_DLLEXPORT bool operator==
    (rifd_entry_point const&, rifd_entry_point const&);
}
