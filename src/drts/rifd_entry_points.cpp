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

#include <drts/private/pimpl.hpp>
#include <drts/private/rifd_entry_points_impl.hpp>

#include <util-generic/read_lines.hpp>

#include <fstream>

namespace gspc
{
  namespace
  {
    std::vector<fhg::rif::entry_point> parse (::boost::filesystem::path const& path)
    {
      std::vector<std::string> const lines (fhg::util::read_lines (path));
      return {lines.begin(), lines.end()};
    }
  }

  rifd_entry_points::rifd_entry_points (::boost::filesystem::path const& path)
    : rifd_entry_points (new implementation (parse (path)))
  {}
  rifd_entry_points::rifd_entry_points (implementation* impl)
    : _ (impl)
  {}
  PIMPL_DTOR (rifd_entry_points)

  rifd_entry_points::rifd_entry_points (rifd_entry_points const& other)
    : _ (new implementation (*other._))
  {}
  rifd_entry_points& rifd_entry_points::operator= (rifd_entry_points const& other)
  {
    _ = std::make_unique<implementation> (*other._);

    return *this;
  }
  rifd_entry_points::rifd_entry_points (rifd_entry_points&&) noexcept = default;
  rifd_entry_points& rifd_entry_points::operator= (rifd_entry_points&&) noexcept = default;

  void rifd_entry_points::write_to_file (::boost::filesystem::path const& path)
  {
    std::ofstream file (path.string());
    if (!file)
    {
      throw std::runtime_error ("unable to open file " + path.string());
    }
    for (fhg::rif::entry_point const& entry_point : _->_entry_points)
    {
      if (!(file << entry_point << '\n'))
      {
        throw std::runtime_error ("unable to write to file " + path.string());
      }
    }
  }

  rifd_entry_point::rifd_entry_point (implementation* impl)
    : _ (impl)
  {}
  rifd_entry_point::rifd_entry_point (rifd_entry_point&& other) noexcept
  {
    _ = std::move (other._);
    other._ = nullptr;
  }
  PIMPL_DTOR (rifd_entry_point)

  std::string const& rifd_entry_point::hostname() const
  {
    return _->hostname();
  }
  std::size_t rifd_entry_point_hash::operator()
    (rifd_entry_point const& ep) const
  {
    return std::hash<fhg::rif::entry_point>() (ep._->_entry_point);
  }
  bool operator== (rifd_entry_point const& lhs, rifd_entry_point const& rhs)
  {
    return lhs._->_entry_point == rhs._->_entry_point;
  }
}
