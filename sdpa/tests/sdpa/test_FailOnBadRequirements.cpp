#define BOOST_TEST_MODULE testFailOnBadRequirements

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (fail_on_invalid_num_workers_required)
{
  // this workflow produces two activities, each requiring 0 workers
  // the job should fail due to invalid number of workers required
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_bad_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (workflow, orchestrator)
    , sdpa::status::FAILED
    );
}
