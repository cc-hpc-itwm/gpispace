#define BOOST_TEST_MODULE testInvalidWorkerNbReq

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace
{
  we::type::activity_t net_with_one_child_requiring_workers (unsigned long count)
  {
    we::type::property::type props;
    props.set ( "fhg.drts.schedule.num_worker"
              , boost::lexical_cast<std::string> (count) + "UL"
              );
    we::type::transition_t transition
      ( fhg::util::random_string()
      , we::type::module_call_t
        (fhg::util::random_string(), fhg::util::random_string())
      , boost::none
      , true
      , props
      , we::priority_type()
      );
    const std::string port_name (fhg::util::random_string());
    we::port_id_type const port_id_in
      ( transition.add_port ( we::type::port_t ( port_name
                                               , we::type::PORT_IN
                                               , std::string ("string")
                                               , we::type::property::type()
                                               )
                            )
      );

    we::type::net_type net;

    we::place_id_type const place_id_in
      (net.add_place (place::type (port_name, std::string ("string"))));

    net.put_value (place_id_in, fhg::util::random_string_without ("\\\""));

    we::transition_id_type const transition_id
      (net.add_transition (transition));

    net.add_connection ( we::edge::PT
                       , transition_id
                       , place_id_in
                       , port_id_in
                       , we::type::property::type()
                       );

    return we::type::activity_t
      ( we::type::transition_t ( fhg::util::random_string()
                               , net
                               , boost::none
                               , true
                               , we::type::property::type()
                               , we::priority_type()
                               )
      , boost::none
      );
  }
}

BOOST_AUTO_TEST_CASE (testInvalidNumberOfWorkersRequired)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (kvs_host(), kvs_port(), orchestrator);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (net_with_one_child_requiring_workers (0).to_string(), orchestrator)
    , sdpa::status::FAILED
    );
}
