// Copyright (C) 2014-2016,2019-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/Activity.hpp>

#include <gspc/we/type/net.hpp>
#include <gspc/we/type/place.hpp>



    namespace gspc::we::type::property
    {
      bool operator== (type const& lhs, type const& rhs);
    }



namespace gspc::we::type::place
{
  bool operator== (type const& lhs, type const& rhs);
}

namespace gspc::we
{
  namespace type
  {
    bool operator== (net_type const& lhs, net_type const& rhs);
    bool operator== (net_type::PlaceIDWithProperty const& lhs, net_type::PlaceIDWithProperty const& rhs);
    bool operator== (net_type::PortIDWithProperty const& lhs, net_type::PortIDWithProperty const& rhs);
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

namespace gspc::we::type
{
  constexpr auto operator== (TokenOnPort const&, TokenOnPort const&) -> bool;
}

#include "detail/operator_equal.ipp"
