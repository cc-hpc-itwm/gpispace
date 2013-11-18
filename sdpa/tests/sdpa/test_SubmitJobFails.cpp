#define BOOST_TEST_MODULE TestSubmitJobFails

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (fail_on_invalid_workflow)
{
  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , ""
    , ""
    , kvs_host(), kvs_port()
    );

  utils::client::submit_job_and_wait_for_termination
    ("invalid workflow", "sdpac", orchestrator);
}

BOOST_AUTO_TEST_CASE (fail_on_empty_workflow)
{
  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , ""
    , ""
    , kvs_host(), kvs_port()
    );

  utils::client::submit_job_and_cancel_and_wait_for_termination
    ("", "sdpac", orchestrator);
}
