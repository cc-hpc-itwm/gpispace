// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/filesystem_lock_directory.hpp>

#include <FMT/boost/filesystem/path.hpp>
#include <exception>
#include <fmt/core.h>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    failed_to_create_lock::failed_to_create_lock
      (::boost::filesystem::path const& path)
        : std::runtime_error
          { fmt::format ("Failed to create lock for {}.", path)
          }
        , _path (path)
    {}

    filesystem_lock_directory::filesystem_lock_directory
      (::boost::filesystem::path const& path)
    try
      : temporary_path (path)
    {}
    catch (...)
    {
      std::throw_with_nested (failed_to_create_lock (path));
    }
  }
}
