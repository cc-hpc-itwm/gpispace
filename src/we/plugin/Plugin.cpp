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

#include <we/plugin/Plugin.hpp>

#include <util-generic/print_exception.hpp>

#include <boost/format.hpp>

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
            (str ( ::boost::format ("Exception in gspc_we_plugin_create: %1%")
                 % fhg::util::current_exception_printer()
                 )
            );
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
