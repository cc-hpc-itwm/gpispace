// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
