// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
