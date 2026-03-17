// Copyright (C) 2010,2013-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/loader/Module.hpp>

#include <filesystem>

#include <list>
#include <mutex>
#include <string>
#include <unordered_map>


  namespace gspc::we::loader
  {
    class loader
    {
    public:
      loader (std::list<std::filesystem::path> const&);

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
      std::list<std::filesystem::path> const _search_path;
    };
  }
