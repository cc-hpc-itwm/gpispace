// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/plugin/Plugin.hpp>

#include <util-generic/print_exception.hpp>

#include <FMT/util-generic/exception_printer.hpp>
#include <exception>
#include <fmt/core.h>
#include <stdexcept>
#include <utility>

namespace gspc
{
  namespace we
  {
    namespace plugin
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
                , fhg::util::exception_printer (std::current_exception())
                )
            };
        }
      }

      Plugin::Plugin ( ::boost::filesystem::path path
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
  }
}
