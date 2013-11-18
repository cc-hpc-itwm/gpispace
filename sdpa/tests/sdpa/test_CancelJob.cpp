#define BOOST_TEST_MODULE TestCancelJob

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

//! \todo This test requires a sleep before canceling the job after
//! submission and is stuck in an endless loop without the sleep. This
//! seems broken.

//! \todo This test uses a coallocation workflow, but has one worker
//! only. Is this intended?
BOOST_AUTO_TEST_CASE (TestCancelCoallocation)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , ""
    //! \todo This is most likely the wrong path!
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  utils::client::submit_job_and_cancel_and_wait_for_termination
    (workflow, "sdpac", orchestrator);
}

BOOST_AUTO_TEST_CASE (Test1)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  utils::client::submit_job_and_cancel_and_wait_for_termination
    (workflow, "sdpac", orchestrator);
}

BOOST_AUTO_TEST_CASE (Test2)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_1
    ( "drts_1", agent
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  utils::client::submit_job_and_cancel_and_wait_for_termination
    (workflow, "sdpac", orchestrator);
}
