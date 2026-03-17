// Copyright (C) 2015,2021-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/drts/pimpl.hpp>
#include <gspc/drts/rifd_entry_points.fwd.hpp>

#include <filesystem>

namespace gspc
{
  class GSPC_EXPORT rifd_entry_points
  {
  public:
    rifd_entry_points (std::filesystem::path const&);

    rifd_entry_points (rifd_entry_points const&);
    rifd_entry_points& operator= (rifd_entry_points const&);
    rifd_entry_points (rifd_entry_points&&) noexcept;
    rifd_entry_points& operator= (rifd_entry_points&&) noexcept;

    void write_to_file (std::filesystem::path const&);

  private:
    friend class rifds;
    friend class scoped_rifds;
    friend class scoped_runtime_system;

    PIMPL (rifd_entry_points);

    rifd_entry_points (implementation*);
  };

  class GSPC_EXPORT rifd_entry_point
  {
  private:
    PIMPL (rifd_entry_point);

  public:
    rifd_entry_point (implementation*);
    rifd_entry_point (rifd_entry_point&&) noexcept;

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

  struct GSPC_EXPORT rifd_entry_point_hash
  {
    std::size_t operator() (rifd_entry_point const&) const;
  };
  GSPC_EXPORT bool operator==
    (rifd_entry_point const&, rifd_entry_point const&);
}
