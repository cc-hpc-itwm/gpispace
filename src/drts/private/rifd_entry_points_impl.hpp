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

#include <drts/rifd_entry_points.hpp>

#include <rif/entry_point.hpp>

#include <vector>

namespace gspc
{
  struct rifd_entry_points::implementation
  {
    implementation (std::vector<fhg::rif::entry_point> const& entry_points)
      : _entry_points (entry_points)
    {}

    std::vector<fhg::rif::entry_point> _entry_points;
  };

  struct rifd_entry_point::implementation
  {
    implementation (fhg::rif::entry_point const& entry_point)
      : _entry_point (entry_point)
    {}
    std::string const& hostname() const
    {
      return _entry_point.hostname;
    }

    fhg::rif::entry_point _entry_point;
  };
}
