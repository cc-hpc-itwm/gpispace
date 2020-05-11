#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <test/certificates_data.hpp>

#include <fhg/util/thread/event.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <functional>
#include <string>

BOOST_DATA_TEST_CASE
  ( restart_workers_while_job_requiring_coallocation_is_running
  , certificates_data
  , certificates
  )
{
  const utils::orchestrator orchestrator (certificates);
  const utils::agent agent (orchestrator, certificates);

  utils::client client (orchestrator, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2)));

  sdpa::worker_id_t worker_id;

  fhg::util::thread::event<std::string> job_submitted_0;
  fhg::util::thread::event<std::string> cancel_requested_0;
  utils::fake_drts_worker_notifying_cancel worker_0
    ( [&] (std::string j) { job_submitted_0.notify (j); }
    , [&cancel_requested_0] (std::string j) { cancel_requested_0.notify (j); }
    , agent
    , certificates
    );

  {
    fhg::util::thread::event<> job_submitted_1;

    const utils::fake_drts_worker_waiting_for_finished_ack worker_1
      ( [&job_submitted_1] (std::string) { job_submitted_1.notify(); }
      , agent
      , certificates
      );

    worker_id = worker_1.name();

    job_submitted_0.wait();
    job_submitted_1.wait();
  }

  worker_0.canceled (cancel_requested_0.wait());

  fhg::util::thread::event<std::string> job_submitted_to_restarted_worker;
  utils::fake_drts_worker_waiting_for_finished_ack restarted_worker
    ( utils::reused_component_name (worker_id)
    , [&job_submitted_to_restarted_worker] (std::string s)
      { job_submitted_to_restarted_worker.notify (s); }
    , agent
    , certificates
    );

  worker_0.finish_and_wait_for_ack (job_submitted_0.wait());
  restarted_worker.finish_and_wait_for_ack
    (job_submitted_to_restarted_worker.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_DATA_TEST_CASE
  ( restart_workers_while_job_is_running_and_partial_result_is_missing
  , certificates_data
  , certificates
  )
{
  const utils::orchestrator orchestrator (certificates);
  const utils::agent agent (orchestrator, certificates);

  utils::client client (orchestrator, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2)));

  sdpa::worker_id_t worker_id;

  fhg::util::thread::event<std::string> job_submitted_0;
  utils::fake_drts_worker_waiting_for_finished_ack worker_0
    ([&job_submitted_0] (std::string j) { job_submitted_0.notify (j); }, agent, certificates);

  {
    fhg::util::thread::event<> job_submitted_1;

    const utils::fake_drts_worker_waiting_for_finished_ack worker_1
      ( [&job_submitted_1] (std::string) { job_submitted_1.notify(); }
      , agent
      , certificates
      );

    worker_id = worker_1.name();

    worker_0.finish_and_wait_for_ack (job_submitted_0.wait());

    job_submitted_1.wait();
  }

  fhg::util::thread::event<std::string> job_submitted_to_restarted_worker;
  utils::fake_drts_worker_waiting_for_finished_ack restarted_worker
    ( utils::reused_component_name (worker_id)
    , [&job_submitted_to_restarted_worker] (std::string s)
      { job_submitted_to_restarted_worker.notify (s); }
    , agent
    , certificates
    );

  worker_0.finish_and_wait_for_ack (job_submitted_0.wait());
  restarted_worker.finish_and_wait_for_ack
    (job_submitted_to_restarted_worker.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
