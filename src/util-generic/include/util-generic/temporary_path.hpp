// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/detail/dllexport.hpp>
#include <boost/filesystem.hpp>
#include <boost/utility.hpp>

#include <stdexcept>
#include <utility>

namespace fhg
{
  namespace util
  {
    namespace error
    {
      struct UTIL_GENERIC_DLLEXPORT path_already_exists : std::logic_error
      {
        ::boost::filesystem::path path;
        path_already_exists (::boost::filesystem::path);
      };
    }

    class temporary_path : ::boost::noncopyable
    {
    public:
      temporary_path (::boost::filesystem::path const& path)
        : _path (::boost::filesystem::absolute (path))
      {
        if (::boost::filesystem::exists (_path))
        {
          throw error::path_already_exists (path);
        }

        ::boost::filesystem::create_directories (_path);
      }
      temporary_path()
        : temporary_path (::boost::filesystem::unique_path())
      {}
      ~temporary_path()
      {
        ::boost::filesystem::remove_all (_path);
      }
      operator ::boost::filesystem::path() const
      {
        return _path;
      }

      temporary_path (temporary_path const&) = delete;
      temporary_path& operator= (temporary_path const&) = delete;
      temporary_path (temporary_path&&) = delete;
      temporary_path& operator= (temporary_path&&) = delete;

    private:
      ::boost::filesystem::path _path;
    };
  }
}
