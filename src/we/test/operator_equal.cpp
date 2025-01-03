// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/test/operator_equal.hpp>

#include <map>
#include <set>

namespace we
{
  namespace type
  {
    namespace property
    {
      bool operator== (type const& lhs, type const& rhs)
      {
        return lhs.list() == rhs.list();
      }
    }
  }
}

namespace place
{
  bool operator== (type const& lhs, type const& rhs)
  {
    return lhs.name() == rhs.name()
      && lhs.signature() == rhs.signature()
      && lhs.property() == rhs.property();
  }
}

namespace we
{
  namespace type
  {
    namespace
    {
      std::map< we::place_id_type
              , std::multiset<pnet::type::value::value_type>
              > tokens (net_type const& n)
      {
        std::map< we::place_id_type
                , std::multiset<pnet::type::value::value_type>
                > tokens_on_place;

        for (auto const& [place_id, _ignore] : n.places())
        {
          for (auto const& [_ignore1, token] : n.get_token (place_id))
          {
            tokens_on_place[place_id].emplace (token);
          }
        }

        return tokens_on_place;
      }
    }

    bool operator== (net_type const& lhs, net_type const& rhs)
    {
      return lhs.places() == rhs.places()
        && lhs.transitions() == rhs.transitions()
        && lhs.transition_to_place() == rhs.transition_to_place()
        && lhs.place_to_transition_consume() == rhs.place_to_transition_consume()
        && lhs.place_to_transition_read() == rhs.place_to_transition_read()
        && lhs.port_to_place() == rhs.port_to_place()
        && lhs.place_to_port() == rhs.place_to_port()
        && tokens (lhs) == tokens (rhs);
    }

    bool operator== (Port const& lhs, Port const& rhs)
    {
      return lhs.name() == rhs.name()
        && lhs.direction() == rhs.direction()
        && lhs.signature() == rhs.signature()
        && lhs.property() == rhs.property();
    }

    bool operator== (Requirement const& lhs, Requirement const& rhs)
    {
      return lhs.value() == rhs.value();
    }

    bool operator== (ModuleCall const& lhs, ModuleCall const& rhs)
    {
      return lhs.module() == rhs.module()
        && lhs.function() == rhs.function();
    }

    bool operator== (Expression const& lhs, Expression const& rhs)
    {
      return lhs.expression() == rhs.expression();
    }

    bool operator== (Transition const& lhs, Transition const& rhs)
    {
      return lhs.name() == rhs.name()
        && lhs.data() == rhs.data()
        && lhs.condition() == rhs.condition()
        && lhs.ports_input() == rhs.ports_input()
        && lhs.ports_output() == rhs.ports_output()
        && lhs.ports_tunnel() == rhs.ports_tunnel()
        && lhs.prop() == rhs.prop()
        && lhs.requirements() == rhs.requirements()
        && lhs.eureka_id() == rhs.eureka_id()
        && lhs.preferences() == rhs.preferences();
    }
  }

    namespace type
    {
      bool operator== (Activity const& lhs, Activity const& rhs)
      {
        return lhs.input() == rhs.input()
          && lhs.output() == rhs.output()
          && lhs.transition() == rhs.transition()
          ;
      }
    }
}

namespace we::type
{
  auto operator==
    ( net_type::PlaceIDWithProperty const& lhs
    , net_type::PlaceIDWithProperty const& rhs
    ) -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie (x._place_id, x._property);
        }
      };

    return essence (lhs) == essence (rhs);
  }
}

namespace we::type
{
  auto operator==
    ( net_type::PortIDWithProperty const& lhs
    , net_type::PortIDWithProperty const& rhs
    ) -> bool
  {
    auto const essence
      { [] (auto const& x)
        {
          return std::tie (x._port_id, x._property);
        }
      };

    return essence (lhs) == essence (rhs);
  }
}
