// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/temporary_path.hpp>

#include <boost/filesystem/path.hpp>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    class failed_to_create_lock : public std::runtime_error
    {
    public:
      failed_to_create_lock (::boost::filesystem::path const&);

      ::boost::filesystem::path const& path() const noexcept
      {
        return _path;
      }

    private:
      ::boost::filesystem::path const _path;
    };

    struct filesystem_lock_directory : public temporary_path
    {
      filesystem_lock_directory (::boost::filesystem::path const&);
    };
  }
}
