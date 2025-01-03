// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/loader/Module.hpp>

#include <we/loader/exceptions.hpp>

#include <util-generic/print_exception.hpp>

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
        (void) fhg::util::scoped_dlhandle (path, flags);
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
