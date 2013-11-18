#define BOOST_TEST_MODULE test_StopRestartAgentsCoalloc

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (restart_drts_worker_while_coallocated_job_is_running)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_one_cap_0
    ( "drts_one_cap_0", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_one_cap_1
    ( "drts_one_cap_1", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  const utils::drts_worker worker_both_caps_0
    ( "drts_both_caps_0", agent
    , "A,B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_both_caps_1
    ( "drts_both_caps_1", agent
    , "A,B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  boost::scoped_ptr<utils::drts_worker> worker_both_caps_to_be_restarted
    ( new utils::drts_worker ( "drts_both_caps_to_be_restarted", agent
                             , "A,B"
                             , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
                             , kvs_host(), kvs_port()
                             )
    );

  boost::thread client_thread
    ( &utils::client::submit_job_and_wait_for_termination_as_subscriber
    , workflow
    , "sdpac"
    , boost::cref (orchestrator)
    );

  worker_both_caps_to_be_restarted.reset();

  //! \note We assume that worker_both_caps_to_be_restarted is
  //! destroyed fast enough, so that the job is not executed yet, but
  //! the client submitted while worker_both_caps_to_be_restarted
  //! still exists and the job actually got scheduled there. In the
  //! end, this is a race.

  utils::drts_worker worker_both_caps_restarted
    ( "drts_both_caps_restarted", agent
    , "A,B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  BOOST_REQUIRE (client_thread.joinable());
  client_thread.join();
}
