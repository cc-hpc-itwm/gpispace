// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace fhg
{
  namespace util
  {
    class temporary_file
    {
    public:
      temporary_file (::boost::filesystem::path const& path)
        : _path (::boost::filesystem::absolute (path))
      {
        if (::boost::filesystem::exists (_path))
        {
          throw std::logic_error
            (( ::boost::format ("Temporary file %1% already exists.")
             % path
             ).str()
            );
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
