// Copyright (C) 2012-2016,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/loader/loader.hpp>

#include <gspc/we/loader/exceptions.hpp>

#include <gspc/util/join.hpp>


  namespace gspc::we::loader
  {
    loader::loader (std::list<std::filesystem::path> const& search_path)
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

      std::filesystem::path const file_name {"lib" + module + ".so"};

      for (std::filesystem::path const& p : _search_path)
      {
        if (std::filesystem::exists (p / file_name))
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
        (file_name.string(), util::join (_search_path, ':').string());
    }
  }
