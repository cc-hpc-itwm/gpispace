// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

#include <we/type/transition.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/require_exception.hpp>

BOOST_DATA_TEST_CASE ( add_connection_only_allows_a_tp_or_tp_many
                     , std::vector<bool> ({true, false})
                     , many_first
                     )
{
  we::type::net_type net;
  we::type::property::type empty;

  we::type::transition_t trans_io
    ( "put_many_dup_test"
      , we::type::expression_t ("")
      , boost::none
      , {}
      , we::priority_type()
    );
  we::transition_id_type const tid (net.add_transition (trans_io));

  we::place_id_type const pid_out_list
    (net.add_place (place::type ("out_list", "list", boost::none)));
  we::place_id_type const pid_out_t
    (net.add_place (place::type ("out", "long", boost::none)));

  we::port_id_type const port_out_list
    ( trans_io.add_port
      (we::type::port_t ("out", we::type::PORT_OUT, "list"))
    );

  auto const edge_first (many_first ? we::edge::TP_MANY : we::edge::TP);
  auto const edge_second (many_first ? we::edge::TP :  we::edge::TP_MANY);
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
