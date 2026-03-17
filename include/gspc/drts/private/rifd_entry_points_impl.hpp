// Copyright (C) 2015-2016,2020,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/rifd_entry_points.hpp>

#include <gspc/rif/entry_point.hpp>

#include <vector>

namespace gspc
{
  struct rifd_entry_points::implementation
  {
    implementation (std::vector<gspc::rif::entry_point> const& entry_points)
      : _entry_points (entry_points)
    {}

    std::vector<gspc::rif::entry_point> _entry_points;
  };

  struct rifd_entry_point::implementation
  {
    implementation (gspc::rif::entry_point const& entry_point)
      : _entry_point (entry_point)
    {}
    std::string const& hostname() const
    {
      return _entry_point.hostname;
    }

    gspc::rif::entry_point _entry_point;
  };
}
