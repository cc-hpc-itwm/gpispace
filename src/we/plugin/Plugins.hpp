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

#pragma once

#include <we/plugin/Plugin.hpp>
#include <we/plugin/ID.hpp>

#include <boost/filesystem/path.hpp>

#include <unordered_map>

namespace gspc
{
  namespace we
  {
    namespace plugin
    {
      struct Plugins
      {
        ID create (::boost::filesystem::path, Context const&, PutToken);
        void destroy (ID);

        void before_eval (ID, Context const&);
        void after_eval (ID, Context const&);

      private:
        ID _next_id = ID {0};
        std::unordered_map<ID, Plugin> _;

        decltype (_)::iterator at (ID);
      };
    }
  }
}
