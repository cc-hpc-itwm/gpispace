// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct C : public Base
      {
        C (Context const& context, PutToken put_token)
        {
          put_token
            ( boost::get<std::string> (context.value ({"B"}))
            , context.value ({"A"})
            );
        }

        virtual void before_eval (Context const&) override
        {
          throw std::runtime_error ("C::before_eval()");
        }
        virtual void after_eval (Context const&) override
        {
          throw std::runtime_error ("C::after_eval()");
        }
      };
    }
  }
}

GSPC_WE_PLUGIN_CREATE (::gspc::we::plugin::C)
