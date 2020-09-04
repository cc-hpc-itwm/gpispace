// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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
      failed_to_create_lock (boost::filesystem::path const&);

      boost::filesystem::path const& path() const noexcept
      {
        return _path;
      }

    private:
      boost::filesystem::path const _path;
    };

    struct filesystem_lock_directory : public temporary_path
    {
      filesystem_lock_directory (boost::filesystem::path const&);
    };
  }
}
