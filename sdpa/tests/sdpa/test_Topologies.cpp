#define BOOST_TEST_MODULE TestTopologies

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (orchestrator_agent_worker)
{
  // O
  // |
  // A
  // |
  // W

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

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (chained_agents)
{
	// O
	// |
	// A
  // |
  // ? -> variable agents #
	// |
	// A
	// |
	// W

  const std::string workflow
    (utils::require_and_read_file ("workflows/transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");

  //! \note "variable agents #" was hardcoded to 1 when this test got
  //! rewritten. Probably should be more, so got bumped to 2.
  const utils::agent<we::mgmt::layer> agent_0
    ("agent_0", "127.0.0.1", orchestrator);
  const utils::agent<we::mgmt::layer> agent_1
    ("agent_1", "127.0.0.1", agent_0);

  const utils::drts_worker worker_0
    ( "drts_0", agent_1
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (two_workers_with_seperate_master_agent)
{
  // O
  // |
  // A-\
  // | |
  // A A
  // | |
  // W W

  const std::string workflow
    (utils::require_and_read_file ("workflows/transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");
  const utils::agent<we::mgmt::layer> agent_0
    ("agent_0", "127.0.0.1", orchestrator);
  const utils::agent<we::mgmt::layer> agent_1
    ("agent_1", "127.0.0.1", agent_0);
  const utils::agent<we::mgmt::layer> agent_2
    ("agent_2", "127.0.0.1", agent_0);

  const utils::drts_worker worker_0
    ( "drts_0", agent_1
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_1
    ( "drts_1", agent_2
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (one_worker_with_multiple_master_agents)
{
  // O-+-\
  // | | |
  // A ? A -> variable agents #
  // | | |
  // W-+-/

  const std::string workflow
    (utils::require_and_read_file ("workflows/transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");

  //! \note "variable agents #" was hardcoded to 1 when this test got
  //! rewritten. Probably should be more, so got bumped to 2.
  const utils::agent<we::mgmt::layer> agent_0
    ("agent_0", "127.0.0.1", orchestrator);
  const utils::agent<we::mgmt::layer> agent_1
    ("agent_1", "127.0.0.1", orchestrator);

  utils::agents_t agents;
  agents.push_back (boost::cref (agent_0));
  agents.push_back (boost::cref (agent_1));

  const utils::drts_worker worker_0
    ( "drts_0", agents
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
                      , sdpa::status::FINISHED
                      );
}

BOOST_AUTO_TEST_CASE (agent_with_multiple_master_agents)
{
  // O-\
  // | |
  // A A
  // | |
  // A-/
  // |
  // W

  const std::string workflow
    (utils::require_and_read_file ("workflows/transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1");

  const utils::agent<we::mgmt::layer> agent_0
    ("agent_0", "127.0.0.1:7700", orchestrator);
  const utils::agent<we::mgmt::layer> agent_1
    ("agent_1", "127.0.0.1:7701", orchestrator);

  utils::agents_t agents;
  agents.push_back (boost::cref (agent_0));
  agents.push_back (boost::cref (agent_1));

  const utils::agent<we::mgmt::layer> agent_2
    ("agent_2", "127.0.0.1:7702", agents);

  const utils::drts_worker worker_0
    ( "drts_0", agent_2
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        (workflow, orchestrator)
                      , sdpa::status::FINISHED
                      );
}
