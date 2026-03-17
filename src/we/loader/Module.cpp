// Copyright (C) 2013-2015,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/loader/Module.hpp>

#include <gspc/we/loader/exceptions.hpp>

#include <gspc/util/print_exception.hpp>

#include <exception>


  namespace gspc::we::loader
  {
    namespace
    {
      auto const flags (RTLD_NOW | RTLD_GLOBAL);

      std::filesystem::path
        ensure_unloads_without_rest_and_load (std::filesystem::path path)
      {
        auto const before (util::currently_loaded_libraries());
        (void) util::scoped_dlhandle (path, flags);
        auto const after (util::currently_loaded_libraries());

        if (before != after)
        {
          throw module_does_not_unload (path, before, after);
        }

        return path;
      }
    }

    Module::Module ( RequireModuleUnloadsWithoutRest
                   , std::filesystem::path const& path
                   )
    try
      : Module (ensure_unloads_without_rest_and_load (path))
    {}
    catch (...)
    {
      throw module_load_failed
        (path, util::current_exception_printer().string());
    }
    Module::Module (std::filesystem::path const& path)
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
        (path, util::current_exception_printer().string());
    }
    void Module::call ( std::string const& function
                      , drts::worker::context *info
                      , we::expr::eval::context const& input
                      , we::expr::eval::context& output
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
