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

#include <we/type/Activity.hpp>

#include <we/type/net.hpp>
#include <we/type/place.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      bool operator== (type const& lhs, type const& rhs);
    }
  }
}

namespace place
{
  bool operator== (type const& lhs, type const& rhs);
}

namespace we
{
  namespace type
  {
    bool operator== (net_type const& lhs, net_type const& rhs);
    bool operator== (Port const& lhs, Port const& rhs);
    bool operator== (Requirement const& lhs, Requirement const& rhs);
    bool operator== (ModuleCall const& lhs, ModuleCall const& rhs);
    bool operator== (Expression const& lhs, Expression const& rhs);
    bool operator== (Transition const& lhs, Transition const& rhs);
  }

    namespace type
    {
      bool operator== (Activity const& lhs, Activity const& rhs);
    }
}
