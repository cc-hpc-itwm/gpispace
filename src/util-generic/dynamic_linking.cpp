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

#include <util-generic/dynamic_linking.hpp>

#include <utility>

#include <link.h>

namespace fhg
{
  namespace util
  {
    scoped_dlhandle::scoped_dlhandle
        (boost::filesystem::path const& path, int flags)
      : _ (syscall::dlopen (path.string().c_str(), flags))
    {}
    scoped_dlhandle::~scoped_dlhandle()
    {
      if (_)
      {
        syscall::dlclose (_);
      }
    }

    scoped_dlhandle::scoped_dlhandle (scoped_dlhandle&& other)
      : _ (std::move (other._))
    {
      other._ = nullptr;
    }

    std::vector<boost::filesystem::path> currently_loaded_libraries()
    {
      std::vector<boost::filesystem::path> result;
      dl_iterate_phdr
        ( +[] (dl_phdr_info* info, size_t, void* data)
           {
             static_cast<std::vector<boost::filesystem::path>*> (data)
               ->emplace_back (info->dlpi_name);
             return 0;
           }
        , &result
        );
      return result;
    }
  }
}
