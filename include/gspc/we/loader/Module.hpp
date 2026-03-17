// Copyright (C) 2010,2013-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/loader/IModule.hpp>

#include <gspc/util/dynamic_linking.hpp>

#include <filesystem>

#include <map>
#include <string>
#include <unordered_map>


  namespace gspc::we::loader
  {
    struct RequireModuleUnloadsWithoutRest{};

    class Module : public IModule
    {
    public:
      Module (std::filesystem::path const& path);
      Module ( RequireModuleUnloadsWithoutRest
             , std::filesystem::path const& path
             );

      ~Module() override = default;
      Module (Module const&) = delete;
      Module (Module&&) = delete;
      Module& operator= (Module const&) = delete;
      Module& operator= (Module&&) = delete;

      void call ( std::string const& f
                , drts::worker::context *context
                , we::expr::eval::context const& in
                , we::expr::eval::context& out
                , std::map<std::string, void*> const& memory_buffer
                ) const;

      void add_function (std::string const&, WrapperFunction) override;

    private:
      std::filesystem::path path_;

      util::scoped_dlhandle _dlhandle;
      std::unordered_map<std::string, WrapperFunction> call_table_;
    };
  }
