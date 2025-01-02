// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <we/type/Transition.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/require_exception.hpp>

BOOST_DATA_TEST_CASE ( add_connection_only_allows_a_tp_or_tp_many
                     , std::vector<bool> ({true, false})
                     , many_first
                     )
{
  we::type::net_type net;
  we::type::property::type empty;

  we::type::Transition trans_io
    ( "put_many_dup_test"
      , we::type::Expression ("")
      , ::boost::none
      , {}
      , we::priority_type()
    , ::boost::optional<we::type::eureka_id_type>{}
    , std::list<we::type::Preference>{}
    );
  we::transition_id_type const tid (net.add_transition (trans_io));

  we::place_id_type const pid_out_list
    (net.add_place (place::type ("out_list", "list", ::boost::none, we::type::property::type{})));
  we::place_id_type const pid_out_t
    (net.add_place (place::type ("out", "long", ::boost::none, we::type::property::type{})));

  we::port_id_type const port_out_list
    ( trans_io.add_port
      (we::type::Port ("out", we::type::port::direction::Out{}, "list", we::type::property::type{}))
    );

  auto const edge_first
    ( many_first
    ? we::edge::type {we::edge::TP_MANY{}}
    : we::edge::type {we::edge::TP{}}
    );
  auto const edge_second
    ( many_first
    ? we::edge::type {we::edge::TP{}}
    : we::edge::type {we::edge::TP_MANY{}}
    );
  auto const pid_first (many_first ? pid_out_t : pid_out_list);
  auto const pid_second (many_first ? pid_out_list : pid_out_t);

  net.add_connection ( edge_first
                     , tid
                     , pid_first
                     , port_out_list
                     , empty
                     );

  fhg::util::testing::require_exception
    ( [&]
      {
        net.add_connection ( edge_second
                           , tid
                           , pid_second
                           , port_out_list
                           , empty
                           );
      }
    , std::logic_error ("duplicate connection: out and out-many")
    );
}
