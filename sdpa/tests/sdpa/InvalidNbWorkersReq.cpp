#define BOOST_TEST_MODULE testInvalidWorkerNbReq

#include <utils.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (setup_logging)

BOOST_AUTO_TEST_CASE (testInvalidNumberOfWorkersRequired)
{
  const utils::orchestrator orchestrator;
  const utils::agent agent (orchestrator);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (utils::net_with_one_child_requiring_workers (0).to_string(), orchestrator)
    , sdpa::status::FAILED
    );
}
