#define BOOST_TEST_MODULE TestCancelJob

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>
#include <cstdlib>
#include <ctime>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (cancel_no_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  sdpa::job_id_t job_id(client.submitJob (workflow));
  client.cancelJob(job_id);

  BOOST_REQUIRE_EQUAL
  ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED );
}

BOOST_AUTO_TEST_CASE (cancel_with_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  sdpa::job_id_t job_id(client.submitJob (workflow));
  while(!agent._.hasJobs());

  client.cancelJob(job_id);

  BOOST_REQUIRE_EQUAL
  ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED );
}

BOOST_AUTO_TEST_CASE (call_cancel_twice_orch)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  sdpa::job_id_t job_id(client.submitJob (workflow));
  client.cancelJob(job_id);

  BOOST_REQUIRE_EQUAL
  ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (call_cancel_twice_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_2", "127.0.0.1", kvs_host(), kvs_port());

  utils::agent agent
      ("agent_2", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  sdpa::client::Client client (orchestrator.name(),  kvs_host(), kvs_port());
  sdpa::job_id_t job_id(client.submitJob (workflow));

  while(!agent._.hasJobs());
  client.cancelJob(job_id);

  BOOST_REQUIRE_EQUAL
  ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}
