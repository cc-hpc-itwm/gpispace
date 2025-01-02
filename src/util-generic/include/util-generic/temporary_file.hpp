// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/detail/dllexport.hpp>
#include <boost/filesystem.hpp>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace error::temporary_file
    {
      auto UTIL_GENERIC_DLLEXPORT already_exists
        ( ::boost::filesystem::path const&
        ) -> std::logic_error
        ;
    }

    class temporary_file
    {
    public:
      temporary_file (::boost::filesystem::path const& path)
        : _path (::boost::filesystem::absolute (path))
      {
        if (::boost::filesystem::exists (_path))
        {
          throw error::temporary_file::already_exists (path);
        }
      }

      ~temporary_file()
      {
        ::boost::filesystem::remove (_path);
      }

      temporary_file (temporary_file const&) = delete;
      temporary_file& operator= (temporary_file const&) = delete;
      temporary_file (temporary_file&&) = delete;
      temporary_file& operator= (temporary_file&&) = delete;

      operator ::boost::filesystem::path() const
      {
        return _path;
      }

    private:
      ::boost::filesystem::path _path;
    };
  }
}
