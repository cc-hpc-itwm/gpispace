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

#include <string>
#include <utility>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct B : public Base
      {
        B (Context const& context, PutToken put_token)
          : _ (::boost::get<std::string> (context.value ({"B"})))
          , _put_token (std::move (put_token))
        {}

        virtual void before_eval (Context const&) override
        {
          _put_token (_, _before++);
        }
        virtual void after_eval (Context const&) override
        {
          _put_token (_, _after++);
        }

      private:
        std::string _;
        PutToken _put_token;
        unsigned _before = 0;
        unsigned _after = 0;
      };
    }
  }
}

GSPC_WE_PLUGIN_CREATE (::gspc::we::plugin::B)
