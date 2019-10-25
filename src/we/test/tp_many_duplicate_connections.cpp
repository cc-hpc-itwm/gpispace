#include <boost/test/unit_test.hpp>

#include <we/type/transition.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

BOOST_AUTO_TEST_CASE (tp_many_check_no_duplicate_tp_many_for_tp)
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

  we::port_id_type const port_out_list
    ( trans_io.add_port
      (we::type::port_t ("out", we::type::PORT_OUT, "list"))
    );

  net.add_connection ( we::edge::TP
                     , tid
                     , pid_out_list
                     , port_out_list
                     , empty
                     );

  we::place_id_type const pid_out_t
    (net.add_place (place::type ("out", "long", boost::none)));

  BOOST_TEST_INFO ("duplicate TP_MANY for existing TP edge");
  fhg::util::testing::require_exception
    ( [&]
      {
        net.add_connection ( we::edge::TP_MANY
                           , tid
                           , pid_out_t
                           , port_out_list
                           , {}
                           );
      }
    , std::logic_error ("duplicate connection: out and out-many")
    );
}

BOOST_AUTO_TEST_CASE (tp_many_check_no_duplicate_tp_for_tp_many)
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

  we::place_id_type const pid_out_t
    (net.add_place (place::type ("out", "long", boost::none)));

  we::port_id_type const port_out_list
    ( trans_io.add_port
      (we::type::port_t ("out", we::type::PORT_OUT, "list"))
    );

  net.add_connection ( we::edge::TP_MANY
                     , tid
                     , pid_out_t
                     , port_out_list
                     , empty
                     );

  we::place_id_type const pid_out_list
    (net.add_place (place::type ("out_list", "list", boost::none)));

  BOOST_TEST_INFO ("duplicate TP for existing TP_MANY edge");
  fhg::util::testing::require_exception
    ( [&]
      {
        net.add_connection ( we::edge::TP
                           , tid
                           , pid_out_list
                           , port_out_list
                           , {}
                           );
      }
    , std::logic_error ("duplicate connection: out and out-many")
    );
}

