// Copyright (C) 2019,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/net.hpp>

#include <gspc/testing/require_exception.hpp>

BOOST_DATA_TEST_CASE ( add_connection_only_allows_a_tp_or_tp_many
                     , std::vector<bool> ({true, false})
                     , many_first
                     )
{
  gspc::we::type::net_type net;
  gspc::we::type::property::type empty;

  gspc::we::type::Transition trans_io
    ( "put_many_dup_test"
      , gspc::we::type::Expression ("")
      , std::nullopt
      , {}
      , gspc::we::priority_type()
    , std::optional<gspc::we::type::eureka_id_type>{}
    , std::list<gspc::we::type::Preference>{}
    , gspc::we::type::track_shared{}
    );
  gspc::we::transition_id_type const tid (net.add_transition (trans_io));

  gspc::we::place_id_type const pid_out_list
    ( net.add_place
        ( gspc::we::type::place::type
            ( "out_list"
            , "list"
            , std::nullopt
            , std::nullopt
            , gspc::we::type::property::type{}
            , gspc::we::type::place::type::Generator::No{}
            )
        )
    );
  gspc::we::place_id_type const pid_out_t
    ( net.add_place
        ( gspc::we::type::place::type
            ( "out"
            , "long"
            , std::nullopt
            , std::nullopt
            , gspc::we::type::property::type{}
            , gspc::we::type::place::type::Generator::No{}
            )
        )
    );

  gspc::we::port_id_type const port_out_list
    ( trans_io.add_port
      (gspc::we::type::Port ("out", gspc::we::type::port::direction::Out{}, "list", gspc::we::type::property::type{}))
    );

  auto const edge_first
    ( many_first
    ? gspc::we::edge::type {gspc::we::edge::TP_MANY{}}
    : gspc::we::edge::type {gspc::we::edge::TP{}}
    );
  auto const edge_second
    ( many_first
    ? gspc::we::edge::type {gspc::we::edge::TP{}}
    : gspc::we::edge::type {gspc::we::edge::TP_MANY{}}
    );
  auto const pid_first (many_first ? pid_out_t : pid_out_list);
  auto const pid_second (many_first ? pid_out_list : pid_out_t);

  net.add_connection ( edge_first
                     , tid
                     , pid_first
                     , port_out_list
                     , empty
                     );

  gspc::testing::require_exception
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
