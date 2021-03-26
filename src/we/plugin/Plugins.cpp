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

#include <we/plugin/Plugins.hpp>

#include <boost/format.hpp>

#include <exception>
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
      ID Plugins::create ( boost::filesystem::path path
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
            (str ( boost::format ("Plugins::create (%1%, %2%)")
                 % path
                 % context
                 )
            )
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
            (str ( boost::format ("Plugins::before_eval (%1%, %2%)")
                 % to_string (pid)
                 % context
                 )
            )
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
            (str ( boost::format ("Plugins::after_eval (%1%, %2%)")
                 % to_string (pid)
                 % context
                 )
            )
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
