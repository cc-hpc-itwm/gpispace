// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/plugin/Plugins.hpp>

#include <FMT/boost/filesystem/path.hpp>
#include <FMT/we/expr/eval/context.hpp>
#include <exception>
#include <fmt/core.h>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      ID Plugins::create ( ::boost::filesystem::path path
                         , Context const& context
                         , PutToken put_token
                         )
      try
      {
        return _.emplace
          ( std::piecewise_construct
          , std::forward_as_tuple (_next_id++)
          , std::forward_as_tuple (path, context, std::move (put_token))
          ).first->first;
      }
      catch (...)
      {
        std::throw_with_nested
          ( std::runtime_error
            { fmt::format ( "Plugins::create ({}, {})"
                          , path
                          , context
                          )
            }
          );
      }

      void Plugins::destroy (ID pid)
      {
        _.erase (at (pid));
      }

      void Plugins::before_eval (ID pid, Context const& context)
      try
      {
        at (pid)->second.before_eval (context);
      }
      catch (...)
      {
        std::throw_with_nested
          ( std::runtime_error
            { fmt::format ( "Plugins::before_eval ({}, {})"
                          , to_string (pid)
                          , context
                          )
            }
          );
      }
      void Plugins::after_eval (ID pid, Context const& context)
      try
      {
        at (pid)->second.after_eval (context);
      }
      catch (...)
      {
        std::throw_with_nested
          ( std::runtime_error
            { fmt::format ( "Plugins::after_eval ({}, {})"
                          , to_string (pid)
                          , context
                          )
            }
          );
      }

      decltype (Plugins::_)::iterator Plugins::at (ID pid)
      {
        auto plugin (_.find (pid));

        if (plugin == _.end())
        {
          throw std::invalid_argument ("Plugins: Unknown " + to_string (pid));
        }

        return plugin;
      }
    }
  }
}
