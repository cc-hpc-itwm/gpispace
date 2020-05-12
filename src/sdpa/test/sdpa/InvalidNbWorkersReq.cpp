#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <test/certificates_data.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

BOOST_DATA_TEST_CASE
  (invalid_number_of_workers_required, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (certificates);
  const utils::agent agent (orchestrator, certificates);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (utils::net_with_one_child_requiring_workers (0), orchestrator, certificates)
    , sdpa::status::FAILED
    );
}
