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

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/utility.hpp>

#include <stdexcept>
#include <utility>

namespace fhg
{
  namespace util
  {
    namespace error
    {
      struct path_already_exists : std::logic_error
      {
        boost::filesystem::path path;
        path_already_exists (decltype (path) p)
          : std::logic_error
              ((boost::format ("Temporary path %1% already exists.") % p).str())
          , path (std::move (p))
        {}
      };
    }

    class temporary_path : boost::noncopyable
    {
    public:
      temporary_path (boost::filesystem::path const& path)
        : _path (boost::filesystem::absolute (path))
      {
        if (boost::filesystem::exists (_path))
        {
          throw error::path_already_exists (path);
        }

        boost::filesystem::create_directories (_path);
      }
      temporary_path()
        : temporary_path (boost::filesystem::unique_path())
      {}
      ~temporary_path()
      {
        boost::filesystem::remove_all (_path);
      }
      operator boost::filesystem::path() const
      {
        return _path;
      }

    private:
      boost::filesystem::path _path;
    };
  }
}
