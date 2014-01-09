#define BOOST_TEST_MODULE testCoallocation

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (test_stalled_when_the_agent_has_no_worker)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/stalled_workflow.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
    ("agent_0", "127.0.0.1", orchestrator);

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id = client.submitJob (workflow);

  sdpa::status::code status(utils::client::wait_for_state_polling(client, job_id, sdpa::status::STALLED));
  BOOST_REQUIRE_EQUAL(status, sdpa::status::STALLED);
}

BOOST_AUTO_TEST_CASE (test_stalled_job_termination_when_new_worker_registers)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/stalled_workflow.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_A_0
    ( "drts_A_0", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_A_1
    ( "drts_A_1", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  const utils::drts_worker worker_B_0
    ( "drts_B_0", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_B_1
    ( "drts_B_1", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id = client.submitJob (workflow);

  // the job should stall because the task A requires 2 workers and the task B requires 3 workers, each
  // and only 2 workers with the capability B are registered
  sdpa::status::code status(utils::client::wait_for_state_polling(client, job_id, sdpa::status::STALLED));
  BOOST_REQUIRE_EQUAL(status, sdpa::status::STALLED);

  // add a new worker having one of the requested capabilities
  // the job should have now enogh workers and the workflow can be
  // entirely executed
  const utils::drts_worker worker_B_2
    ( "drts_B_2", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );


  sdpa::client::job_info_t job_info;
  status = client.wait_for_terminal_state (job_id, job_info);
  BOOST_REQUIRE_EQUAL(status, sdpa::status::FINISHED);
}

BOOST_AUTO_TEST_CASE (test_stalled_job_termination_when_worker_gains_cpb)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/stalled_workflow.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
    ("agent_0", "127.0.0.1", orchestrator);

  const utils::drts_worker worker_A_0
    ( "drts_A_0", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_A_1
    ( "drts_A_1", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  const utils::drts_worker worker_B_0
    ( "drts_B_0", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_B_1
    ( "drts_B_1", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  utils::drts_worker worker_B_2
    ( "drts_B_2", agent
    , ""
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id = client.submitJob (workflow);

  // the job should stall because the task A requires 2 workers and the task B requires 3 workers, each
  // and only 2 workers with the capability B are registered
  sdpa::status::code status(utils::client::wait_for_state_polling(client, job_id, sdpa::status::STALLED));
  BOOST_REQUIRE_EQUAL(status, sdpa::status::STALLED);

  // add a new worker having one of the requested capabilities
  // the job should have now enogh workers and the workflow can be
  // entirely executed
  worker_B_2.add_capability("B");

  sdpa::client::job_info_t job_info;
  status = client.wait_for_terminal_state (job_id, job_info);
  BOOST_REQUIRE_EQUAL(status, sdpa::status::FINISHED);
}

BOOST_AUTO_TEST_CASE (test_stalled_job_termination_when_workers_are_progressively_added)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/stalled_workflow.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent agent
    ("agent_0", "127.0.0.1", orchestrator);

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id = client.submitJob (workflow);

  sdpa::status::code status = utils::client::wait_for_state_polling(client, job_id, sdpa::status::STALLED);
  BOOST_REQUIRE_EQUAL(status, sdpa::status::STALLED);

  const utils::drts_worker worker_A_0
    ( "drts_A_0", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  status = utils::client::wait_for_state_polling(client, job_id, sdpa::status::STALLED);
  BOOST_REQUIRE_EQUAL(status, sdpa::status::STALLED);

  const utils::drts_worker worker_A_1
    ( "drts_A_1", agent
    , "A"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  status = utils::client::wait_for_state_polling(client, job_id, sdpa::status::STALLED);
  BOOST_REQUIRE_EQUAL(status, sdpa::status::STALLED);

  const utils::drts_worker worker_B_0
    ( "drts_B_0", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  status = utils::client::wait_for_state_polling(client, job_id, sdpa::status::STALLED);
  BOOST_REQUIRE_EQUAL(status, sdpa::status::STALLED);

  const utils::drts_worker worker_B_1
    ( "drts_B_1", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  status = utils::client::wait_for_state_polling(client, job_id, sdpa::status::STALLED);
  BOOST_REQUIRE_EQUAL(status, sdpa::status::STALLED);

  const utils::drts_worker worker_B_2
    ( "drts_B_2", agent
    , "B"
    , TESTS_EXAMPLE_COALLOCATION_TEST_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  sdpa::client::job_info_t job_info;
  status = client.wait_for_terminal_state (job_id, job_info);
  BOOST_REQUIRE_EQUAL(status, sdpa::status::FINISHED);
}
