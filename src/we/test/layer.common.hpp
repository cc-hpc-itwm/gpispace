// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/layer.hpp>
#include <we/type/Activity.hpp>
#include <we/type/ModuleCall.hpp>
#include <we/type/Transition.hpp>
#include <we/type/signature.hpp>
#include <we/type/value/read.hpp>
#include <we/type/value/wrap.hpp>

namespace
{
  namespace value
  {
    pnet::type::value::value_type const CONTROL
    (pnet::type::value::read ("[]"));
  }
  namespace signature
  {
    pnet::type::signature::signature_type CONTROL (std::string ("control"));
    pnet::type::signature::signature_type LONG (std::string ("long"));
    pnet::type::signature::signature_type SET (std::string ("SET"));
  }
}

namespace
{
  std::tuple< we::type::Transition
            , we::type::Transition
            , we::transition_id_type
            >
    net_with_childs (bool put_on_input, std::size_t token_count)
  {
    we::type::Transition transition
      ( "module call"
      , we::type::ModuleCall
        ( "m"
        , "f"
        , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
        , std::list<we::type::memory_transfer>()
        , std::list<we::type::memory_transfer>()
        , true
        , true
        )
      , ::boost::none
      , we::type::property::type()
      , we::priority_type()
      , ::boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );
    we::port_id_type const port_id_in
      ( transition.add_port ( we::type::Port ( "in"
                                               , we::type::port::direction::In{}
                                               , signature::CONTROL
                                               , we::type::property::type()
                                               )
                            )
      );
    we::port_id_type const port_id_out
      ( transition.add_port ( we::type::Port ( "out"
                                               , we::type::port::direction::Out{}
                                               , signature::CONTROL
                                               , we::type::property::type()
                                               )
                            )
      );

    we::type::net_type net;

    we::place_id_type const place_id_in
      (net.add_place (place::type ("in", signature::CONTROL, true, we::type::property::type{})));
    we::place_id_type const place_id_out
      (net.add_place (place::type ("out", signature::CONTROL, ::boost::none, we::type::property::type{})));

    for (std::size_t i (0); i < token_count; ++i)
    {
      net.put_value (put_on_input ? place_id_in : place_id_out, value::CONTROL);
    }

    we::transition_id_type const transition_id
      (net.add_transition (transition));

    {
      we::type::property::type empty;

      net.add_connection (we::edge::TP{}, transition_id, place_id_out, port_id_out, empty);
      net.add_connection (we::edge::PT{}, transition_id, place_id_in, port_id_in, empty);
    }

    return std::make_tuple
      ( we::type::Transition ( "net"
                               , net
                               , ::boost::none
                               , we::type::property::type()
                               , we::priority_type()
                               , ::boost::optional<we::type::eureka_id_type>{}
                               , std::list<we::type::Preference>{}
                               )
      , transition
      , transition_id
      );
  }

  std::tuple< we::type::Activity
            , we::type::Activity
            , we::type::Activity
            , we::type::Activity
            >
    activity_with_child (std::size_t token_count)
  {
    we::transition_id_type transition_id_child;
    we::type::Transition transition_in;
    we::type::Transition transition_out;
    we::type::Transition transition_child;
    std::tie (transition_in, transition_child, transition_id_child) =
      net_with_childs (true, token_count);
    std::tie (transition_out, std::ignore, std::ignore) =
      net_with_childs (false, token_count);

    we::type::Activity activity_input (transition_in);
    we::type::Activity activity_output (transition_out);

    we::type::Activity activity_child (transition_child);
    activity_child.add_input ("in", value::CONTROL);

    we::type::Activity activity_result
      (we::type::TESTING_ONLY{}, transition_child, transition_id_child);
    activity_result.add_output_TESTING_ONLY ("out", value::CONTROL);

    return std::make_tuple
      (activity_input, activity_output, activity_child, activity_result);
  }
}
