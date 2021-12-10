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

#include <we/loader/IModule.hpp>

#include <util-generic/dynamic_linking.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/utility.hpp>

#include <string>
#include <unordered_map>
#include <map>

namespace we
{
  namespace loader
  {
    struct RequireModuleUnloadsWithoutRest{};

    class Module : public IModule, ::boost::noncopyable
    {
    public:
      Module (::boost::filesystem::path const& path);
      Module ( RequireModuleUnloadsWithoutRest
             , ::boost::filesystem::path const& path
             );

      void call ( std::string const& f
                , drts::worker::context *context
                , expr::eval::context const& in
                , expr::eval::context& out
                , std::map<std::string, void*> const& memory_buffer
                ) const;

      virtual void add_function (std::string const&, WrapperFunction) override;

    private:
      ::boost::filesystem::path path_;

      fhg::util::scoped_dlhandle _dlhandle;
      std::unordered_map<std::string, WrapperFunction> call_table_;
    };
  }
}
