// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/loader/IModule.hpp>

#include <util-generic/dynamic_linking.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/utility.hpp>

#include <map>
#include <string>
#include <unordered_map>

namespace we
{
  namespace loader
  {
    struct RequireModuleUnloadsWithoutRest{};

    class Module : public IModule
    {
    public:
      Module (::boost::filesystem::path const& path);
      Module ( RequireModuleUnloadsWithoutRest
             , ::boost::filesystem::path const& path
             );

      ~Module() override = default;
      Module (Module const&) = delete;
      Module (Module&&) = delete;
      Module& operator= (Module const&) = delete;
      Module& operator= (Module&&) = delete;

      void call ( std::string const& f
                , drts::worker::context *context
                , expr::eval::context const& in
                , expr::eval::context& out
                , std::map<std::string, void*> const& memory_buffer
                ) const;

      void add_function (std::string const&, WrapperFunction) override;

    private:
      ::boost::filesystem::path path_;

      fhg::util::scoped_dlhandle _dlhandle;
      std::unordered_map<std::string, WrapperFunction> call_table_;
    };
  }
}
