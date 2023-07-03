// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/filesystem_lock_directory.hpp>

#include <boost/format.hpp>

#include <exception>

namespace fhg
{
  namespace util
  {
    failed_to_create_lock::failed_to_create_lock
      (::boost::filesystem::path const& path)
        : std::runtime_error
          ( ( ::boost::format ("Failed to create lock for %1%.")
            % path
            ).str()
          )
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
