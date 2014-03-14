#define BOOST_TEST_MODULE TestSubmitJobFails

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

BOOST_AUTO_TEST_CASE (fail_on_invalid_workflow)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        ("invalid workflow", orchestrator)
                      , sdpa::status::FAILED
                      );
}

BOOST_AUTO_TEST_CASE (fail_on_empty_workflow)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        ("", orchestrator)
                      , sdpa::status::FAILED
                      );
}
