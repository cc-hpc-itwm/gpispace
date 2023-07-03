// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
    class loader
    {
    public:
      loader (std::list<::boost::filesystem::path> const&);

      ~loader() = default;
      loader (loader const&) = delete;
      loader (loader&&) = delete;
      loader& operator= (loader const&) = delete;
      loader& operator= (loader&&) = delete;

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
      std::list<::boost::filesystem::path> const _search_path;
    };
  }
}
