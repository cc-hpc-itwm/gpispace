#pragma once

#include <we/layer.hpp>
#include <we/type/activity.hpp>
#include <we/type/module_call.hpp>
#include <we/type/signature.hpp>
#include <we/type/transition.hpp>
#include <we/type/value/read.hpp>

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
  }
}

namespace
{
  std::tuple< we::type::transition_t
            , we::type::transition_t
            , we::transition_id_type
            >
    net_with_childs (bool put_on_input, std::size_t token_count)
  {
    we::type::transition_t transition
      ( "module call"
      , we::type::module_call_t
        ( "m"
        , "f"
        , std::unordered_map<std::string, std::string>()
        , std::list<we::type::memory_transfer>()
        , std::list<we::type::memory_transfer>()
        )
      , boost::none
      , we::type::property::type()
      , we::priority_type()
      );
    we::port_id_type const port_id_in
      ( transition.add_port ( we::type::port_t ( "in"
                                               , we::type::PORT_IN
                                               , signature::CONTROL
                                               , we::type::property::type()
                                               )
                            )
      );
    we::port_id_type const port_id_out
      ( transition.add_port ( we::type::port_t ( "out"
                                               , we::type::PORT_OUT
                                               , signature::CONTROL
                                               , we::type::property::type()
                                               )
                            )
      );

    we::type::net_type net;

    we::place_id_type const place_id_in
      (net.add_place (place::type ("in", signature::CONTROL, true)));
    we::place_id_type const place_id_out
      (net.add_place (place::type ("out", signature::CONTROL, boost::none)));

    for (std::size_t i (0); i < token_count; ++i)
    {
      net.put_value (put_on_input ? place_id_in : place_id_out, value::CONTROL);
    }

    we::transition_id_type const transition_id
      (net.add_transition (transition));

    {
      using we::edge::TP;
      using we::edge::PT;
      we::type::property::type empty;

      net.add_connection (TP, transition_id, place_id_out, port_id_out, empty);
      net.add_connection (PT, transition_id, place_id_in, port_id_in, empty);
    }

    return std::make_tuple
      ( we::type::transition_t ( "net"
                               , net
                               , boost::none
                               , we::type::property::type()
                               , we::priority_type()
                               )
      , transition
      , transition_id
      );
  }

  std::tuple< we::type::activity_t
            , we::type::activity_t
            , we::type::activity_t
            , we::type::activity_t
            >
    activity_with_child (std::size_t token_count)
  {
    we::transition_id_type transition_id_child;
    we::type::transition_t transition_in;
    we::type::transition_t transition_out;
    we::type::transition_t transition_child;
    std::tie (transition_in, transition_child, transition_id_child) =
      net_with_childs (true, token_count);
    std::tie (transition_out, std::ignore, std::ignore) =
      net_with_childs (false, token_count);

    we::type::activity_t activity_input (transition_in, boost::none);
    we::type::activity_t activity_output (transition_out, boost::none);

    we::type::activity_t activity_child
      (transition_child, transition_id_child);
    activity_child.add_input
      (transition_child.input_port_by_name ("in"), value::CONTROL);

    we::type::activity_t activity_result
      (transition_child, transition_id_child);
    activity_result.add_output
      (transition_child.output_port_by_name ("out"), value::CONTROL);

    return std::make_tuple
      (activity_input, activity_output, activity_child, activity_result);
  }
}
