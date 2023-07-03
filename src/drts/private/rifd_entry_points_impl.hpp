// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
