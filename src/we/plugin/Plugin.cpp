// Copyright (C) 2019,2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/plugin/Plugin.hpp>

#include <gspc/util/print_exception.hpp>

#include <gspc/util/exception_printer.formatter.hpp>
#include <exception>
#include <fmt/core.h>
#include <stdexcept>
#include <utility>



    namespace gspc::we::plugin
    {
      namespace
      {
        template<typename Function, typename... Args>
          auto call_and_extract_message_of_potential_exception
            ( Function&& function
            , Args&&... args
            )
        try
        {
          return function (std::forward<Args> (args)...);
        }
        catch (...)
        {
          throw std::runtime_error
            { fmt::format
                ( "Exception in gspc_we_plugin_create: {}"
                , util::exception_printer (std::current_exception())
                )
            };
        }
      }

      Plugin::Plugin ( std::filesystem::path path
                     , Context const& context
                     , PutToken put_token
                     )
        : _dlhandle (path)
        , _ ( call_and_extract_message_of_potential_exception
              ( FHG_UTIL_SCOPED_DLHANDLE_SYMBOL
                  (_dlhandle, gspc_we_plugin_create)
              , context
              , std::move (put_token)
              )
            )
      {}

      void Plugin::before_eval (Context const& context)
      {
        _->before_eval (context);
      }
      void Plugin::after_eval (Context const& context)
      {
        _->after_eval (context);
      }
    }
