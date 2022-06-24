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

#include <we/plugin/Base.hpp>

#include <stdexcept>
#include <string>
#include <utility>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct Tunnel : public Base
      {
        Tunnel (Context const& context, PutToken put_token)
          : _place_name
            ( ::boost::get<std::string>
                (context.value ({"plugin", "Tunnel", "place_name"}))
            )
          , _context_key
            ( ::boost::get<std::string>
                (context.value ({"plugin", "Tunnel", "context_key"}))
            )
          , _put_token (std::move (put_token))
        {}

        void before_eval (Context const&) override
        {
          throw std::logic_error ("Tunnel::before_eval");
        }
        void after_eval (Context const& context) override
        {
          return _put_token (_place_name, context.value ({_context_key}));
        }

      private:
        std::string _place_name;
        std::string _context_key;
        PutToken _put_token;
      };
    }
  }
}

GSPC_WE_PLUGIN_CREATE (gspc::we::plugin::Tunnel)
