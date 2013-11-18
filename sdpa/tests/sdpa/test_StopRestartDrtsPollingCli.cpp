#define BOOST_TEST_MODULE TestStopRestartDrtsPollingCli

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (restart_drts_worker_while_job_is_running)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent
    ("agent_0", "127.0.0.1", orchestrator);

  boost::scoped_ptr<utils::drts_worker> worker_0
    ( new utils::drts_worker ( "drts_0", agent
                             , ""
                             , TESTS_TRANSFORM_FILE_MODULES_PATH
                             , kvs_host(), kvs_port()
                             )
    );

  boost::thread client_thread
    ( &utils::client::submit_job_and_wait_for_termination
    , workflow
    , "sdpac"
    , boost::cref (orchestrator)
    );

  worker_0.reset();

  //! \note We hope that worker_0 is destroyed fast enough, so that
  //! the job is not executed yet, but the client submitted while
  //! worker_0 still exists. Most likely we are just testing if it
  //! will wait for some worker to be able to process the workflow. In
  //! the end, this is a race.

  const utils::drts_worker worker_new
    ( "drts_new", agent
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  BOOST_REQUIRE (client_thread.joinable());
  client_thread.join();
}
