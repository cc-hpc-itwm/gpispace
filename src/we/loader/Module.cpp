// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <we/loader/Module.hpp>

#include <we/loader/exceptions.hpp>

#include <util-generic/print_exception.hpp>

#include <boost/format.hpp>

#include <exception>

namespace we
{
  namespace loader
  {
    namespace
    {
      auto const flags (RTLD_NOW | RTLD_GLOBAL);

      ::boost::filesystem::path
        ensure_unloads_without_rest_and_load (::boost::filesystem::path path)
      {
        auto const before (fhg::util::currently_loaded_libraries());
        fhg::util::scoped_dlhandle (path, flags);
        auto const after (fhg::util::currently_loaded_libraries());

        if (before != after)
        {
          throw module_does_not_unload (path, before, after);
        }

        return path;
      }
    }

    Module::Module ( RequireModuleUnloadsWithoutRest
                   , ::boost::filesystem::path const& path
                   )
    try
      : Module (ensure_unloads_without_rest_and_load (path))
    {}
    catch (...)
    {
      throw module_load_failed
        (path, fhg::util::current_exception_printer().string());
    }
    Module::Module (::boost::filesystem::path const& path)
    try
      : path_ (path)
      , _dlhandle (path, flags)
      , call_table_()
    {
      _dlhandle.sym<void (IModule*)> ("we_mod_initialize") (this);
    }
    catch (...)
    {
      throw module_load_failed
        (path, fhg::util::current_exception_printer().string());
    }
    void Module::call ( std::string const& function
                      , drts::worker::context *info
                      , expr::eval::context const& input
                      , expr::eval::context& output
                      , std::map<std::string, void*> const& memory_buffer
                      ) const
    {
      const std::unordered_map<std::string, WrapperFunction>::const_iterator
        fun (call_table_.find (function));

      if (fun == call_table_.end())
      {
        throw function_not_found (path_, function);
      }

      (*fun->second)(info, input, output, memory_buffer);
    }
    void Module::add_function (std::string const& name, WrapperFunction f)
    {
      if (! call_table_.emplace (name, f).second)
      {
        throw duplicate_function (path_, name);
      }
    }
  }
}
