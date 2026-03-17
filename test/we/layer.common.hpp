// Copyright (C) 2014-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/layer.hpp>
#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/ModuleCall.hpp>
#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/signature.hpp>
#include <gspc/we/type/value/read.hpp>
#include <gspc/we/type/value/wrap.hpp>

namespace
{
  namespace value
  {
    gspc::pnet::type::value::value_type const CONTROL
    (gspc::pnet::type::value::read ("[]"));
  }
  namespace signature
  {
    gspc::pnet::type::signature::signature_type CONTROL (std::string ("control"));
    gspc::pnet::type::signature::signature_type LONG (std::string ("long"));
    gspc::pnet::type::signature::signature_type SET (std::string ("SET"));
  }
}

namespace
{
  std::tuple< gspc::we::type::Transition
            , gspc::we::type::Transition
            , gspc::we::transition_id_type
            >
    net_with_childs (bool put_on_input, std::size_t token_count)
  {
    gspc::we::type::Transition transition
      ( "module call"
      , gspc::we::type::ModuleCall
        ( "m"
        , "f"
        , std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>()
        , std::list<gspc::we::type::memory_transfer>()
        , std::list<gspc::we::type::memory_transfer>()
        , true
        , true
        )
      , std::nullopt
      , gspc::we::type::property::type()
      , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
    gspc::we::port_id_type const port_id_in
      ( transition.add_port ( gspc::we::type::Port ( "in"
                                               , gspc::we::type::port::direction::In{}
                                               , signature::CONTROL
                                               , gspc::we::type::property::type()
                                               )
                            )
      );
    gspc::we::port_id_type const port_id_out
      ( transition.add_port ( gspc::we::type::Port ( "out"
                                               , gspc::we::type::port::direction::Out{}
                                               , signature::CONTROL
                                               , gspc::we::type::property::type()
                                               )
                            )
      );

    gspc::we::type::net_type net;

    gspc::we::place_id_type const place_id_in
      ( net.add_place
          ( gspc::we::type::place::type
              ( "in"
              , signature::CONTROL
              , true
              , std::nullopt
              , gspc::we::type::property::type{}
              , gspc::we::type::place::type::Generator::No{}
              )
          )
      );
    gspc::we::place_id_type const place_id_out
      ( net.add_place
          ( gspc::we::type::place::type
              ( "out"
              , signature::CONTROL
              , std::nullopt
              , std::nullopt
              , gspc::we::type::property::type{}
              , gspc::we::type::place::type::Generator::No{}
              )
          )
      );

    for (std::size_t i (0); i < token_count; ++i)
    {
      net.put_value (put_on_input ? place_id_in : place_id_out, value::CONTROL);
    }

    gspc::we::transition_id_type const transition_id
      (net.add_transition (transition));

    {
      gspc::we::type::property::type empty;

      net.add_connection (gspc::we::edge::TP{}, transition_id, place_id_out, port_id_out, empty);
      net.add_connection (gspc::we::edge::PT{}, transition_id, place_id_in, port_id_in, empty);
    }

    return std::make_tuple
      ( gspc::we::type::Transition ( "net"
                               , net
                               , std::nullopt
                               , gspc::we::type::property::type()
                               , gspc::we::priority_type()
                               , std::optional<gspc::we::type::eureka_id_type>{}
                               , std::list<gspc::we::type::Preference>{}
                               , gspc::we::type::track_shared{}
                               )
      , transition
      , transition_id
      );
  }

  std::tuple< gspc::we::type::Activity
            , gspc::we::type::Activity
            , gspc::we::type::Activity
            , gspc::we::type::Activity
            >
    activity_with_child (std::size_t token_count)
  {
    gspc::we::transition_id_type transition_id_child;
    gspc::we::type::Transition transition_in;
    gspc::we::type::Transition transition_out;
    gspc::we::type::Transition transition_child;
    std::tie (transition_in, transition_child, transition_id_child) =
      net_with_childs (true, token_count);
    std::tie (transition_out, std::ignore, std::ignore) =
      net_with_childs (false, token_count);

    gspc::we::type::Activity activity_input (transition_in);
    gspc::we::type::Activity activity_output (transition_out);

    gspc::we::type::Activity activity_child (transition_child);
    activity_child.add_input ("in", value::CONTROL);

    gspc::we::type::Activity activity_result
      (gspc::we::type::TESTING_ONLY{}, transition_child, transition_id_child);
    activity_result.add_output_TESTING_ONLY ("out", value::CONTROL);

    return std::make_tuple
      (activity_input, activity_output, activity_child, activity_result);
  }
}
