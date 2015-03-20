#define BOOST_TEST_MODULE TestSubmitJobFails

#include <utils.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_CASE (fail_on_invalid_workflow, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator, _logger);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        ("invalid workflow", orchestrator)
                      , sdpa::status::FAILED
                      );
}

BOOST_FIXTURE_TEST_CASE (fail_on_empty_workflow, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator, _logger);

  BOOST_REQUIRE_EQUAL ( utils::client::submit_job_and_wait_for_termination
                        ("", orchestrator)
                      , sdpa::status::FAILED
                      );
}
