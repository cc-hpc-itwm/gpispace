#define BOOST_TEST_MODULE testInvalidWorkerNbReq

#include <utils.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_CASE (testInvalidNumberOfWorkersRequired, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator, _logger);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (utils::net_with_one_child_requiring_workers (0).to_string(), orchestrator)
    , sdpa::status::FAILED
    );
}
