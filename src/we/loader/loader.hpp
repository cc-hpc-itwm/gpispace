// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <we/loader/Module.hpp>

#include <boost/filesystem.hpp>

#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

namespace we
{
  namespace loader
  {
    class loader : boost::noncopyable
    {
    public:
      loader (std::list<boost::filesystem::path> const&);

      Module const& operator[] (std::string const& m)
      {
        return module (false, m);
      }
      Module const& module
        ( bool require_module_unloads_without_rest
        , std::string const& module
        );

   private:
      std::mutex _table_mutex;
      std::unordered_map<std::string, std::unique_ptr<Module>> _module_table;
      std::list<boost::filesystem::path> const _search_path;
    };
  }
}
