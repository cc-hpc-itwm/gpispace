#include <utils.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

void testInvalidNumberOfWorkersRequired
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  const utils::orchestrator orchestrator (_logger, certificates);
  const utils::agent agent (orchestrator, _logger, certificates);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (utils::net_with_one_child_requiring_workers (0), orchestrator, certificates)
    , sdpa::status::FAILED
    );
}

BOOST_FIXTURE_TEST_CASE (InvalidNumberOfWorkersRequired, setup_logging)
{
  testInvalidNumberOfWorkersRequired (_logger, boost::none);
}

BOOST_FIXTURE_TEST_CASE
  (InvalidNumberOfWorkersRequired_using_secure_communication, setup_logging)
{
  testInvalidNumberOfWorkersRequired (_logger, test_certificates);
}
