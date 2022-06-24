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
