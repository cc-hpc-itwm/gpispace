#define BOOST_TEST_MODULE testCoallocation

#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>

BOOST_GLOBAL_FIXTURE (setup_logging)

BOOST_AUTO_TEST_CASE (testCoallocationWorkflow)
{
  const utils::kvs_server kvs_server;
  const utils::orchestrator orchestrator (kvs_server);
  const utils::agent agent (orchestrator);

  const utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent);
  const utils::fake_drts_worker_directly_finishing_jobs worker_1 (agent);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (utils::net_with_one_child_requiring_workers (2).to_string(), orchestrator)
    , sdpa::status::FINISHED
    );
}
