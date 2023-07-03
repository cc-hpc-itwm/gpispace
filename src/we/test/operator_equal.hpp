// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
