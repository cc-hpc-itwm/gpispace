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

#include <we/loader/loader.hpp>

#include <we/loader/exceptions.hpp>

#include <util-generic/join.hpp>

namespace we
{
  namespace loader
  {
    loader::loader (std::list<::boost::filesystem::path> const& search_path)
      : _search_path (search_path)
    {}

    Module const& loader::module
      ( bool require_module_unloads_without_rest
      , std::string const& module
      )
    {
      std::lock_guard<std::mutex> const _ (_table_mutex);

      std::unordered_map<std::string, std::unique_ptr<Module>>::const_iterator
        const mod (_module_table.find (module));

      if (mod != _module_table.end())
      {
        return *mod->second;
      }

      const ::boost::filesystem::path file_name ("lib" + module + ".so");

      for (::boost::filesystem::path const& p : _search_path)
      {
        if (::boost::filesystem::exists (p / file_name))
        {
          return *_module_table
            .emplace ( module
                     , require_module_unloads_without_rest
                     ? std::make_unique<Module>
                         (RequireModuleUnloadsWithoutRest{}, p / file_name)
                     : std::make_unique<Module>
                         (p / file_name)
                     )
            .first->second;
        }
      }

      throw module_not_found
        (file_name.string(), fhg::util::join (_search_path, ':').string());
    }
  }
}
