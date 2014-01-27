#define BOOST_TEST_MODULE TestCancelJob

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (test_cancel_no_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test2.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");

  sdpa::client::Client client (orchestrator.name());

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_cancel_and_wait_for_termination
      (workflow, orchestrator)
    , sdpa::status::CANCELED
    );
}

BOOST_AUTO_TEST_CASE (test_cance_orch_and_agent_no_worker)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
    ("agent_0", "127.0.0.1", orchestrator);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_cancel_and_wait_for_termination
      (workflow, orchestrator)
    , sdpa::status::CANCELED
    );
}

BOOST_AUTO_TEST_CASE (test_call_cancel_twice)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test2.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
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

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id(client.submitJob (workflow));
  client.cancelJob(job_id);
  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
    ( client.wait_for_terminal_state (job_id, job_info)
    , sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (test_call_cancel_with_timeout)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/capabilities.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
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

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id(client.submitJob (workflow));

  // wait some time before canceling the job
  static const boost::posix_time::milliseconds timeout(100);
  boost::this_thread::sleep(timeout);

  client.cancelJob(job_id);
  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
    ( client.wait_for_terminal_state (job_id, job_info)
        , sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (test_call_cancel_with_polling_client)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/capabilities.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
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

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id(client.submitJob (workflow));

  client.cancelJob(job_id);
  sdpa::client::job_info_t job_info;
  client.wait_for_terminal_state_polling (job_id, job_info);
  BOOST_REQUIRE_EQUAL(client.queryJob(job_id), sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (test_cancel_terminated_job)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/capabilities.pnet"));

  const utils::orchestrator orchestrator ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , "A"
    , TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_1
    ( "drts_1", agent
    , "B"
    , TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_2
    ( "drts_2", agent
    , "A"
    , TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id(client.submitJob (workflow));

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
    ( client.wait_for_terminal_state (job_id, job_info)
        , sdpa::status::FINISHED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}
