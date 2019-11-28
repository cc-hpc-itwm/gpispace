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

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

namespace
{
  class fake_drts_worker_waiting_for_cancel final
    : public utils::no_thread::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_waiting_for_cancel
        ( std::function<void (std::string)> announce_job
        , const utils::agent& master_agent
        , fhg::com::Certificates const& certificates
        )
      : utils::no_thread::fake_drts_worker_notifying_module_call_submission
        (announce_job, master_agent, certificates)
      , _pending_cancel_requests (0)
    {}
    ~fake_drts_worker_waiting_for_cancel()
    {
      BOOST_REQUIRE_EQUAL (_pending_cancel_requests, 0);
    }

    void handleCancelJobEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent* pEvt) override
    {
      _network.perform<sdpa::events::CancelJobAckEvent>
        (source, pEvt->job_id());

      std::lock_guard<std::mutex> const _ (_mtx_cancel);
      ++_pending_cancel_requests;
      _cond_cancel.notify_one();
    }

    void wait_for_cancel()
    {
      std::unique_lock<std::mutex> lock (_mtx_cancel);
      _cond_cancel.wait
        (lock, [&] { return _pending_cancel_requests == 1; });
      --_pending_cancel_requests;
    }

  private:
    std::mutex _mtx_cancel;
    std::condition_variable _cond_cancel;
    std::size_t _pending_cancel_requests;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };
}

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

  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  fhg::util::thread::event<std::string> job_submitted_0;
  fake_drts_worker_waiting_for_cancel worker_0
    ([&job_submitted_0] (std::string j) { job_submitted_0.notify (j); }, agent, certificates);

  {
    fhg::util::thread::event<> job_submitted_1;

    const utils::fake_drts_worker_notifying_module_call_submission worker_1
      ( worker_id
      , [&job_submitted_1] (std::string) { job_submitted_1.notify(); }
      , agent
      , certificates
      );

    job_submitted_0.wait();
    job_submitted_1.wait();
  }

  worker_0.wait_for_cancel();

  const utils::fake_drts_worker_directly_finishing_jobs restarted_worker
    (worker_id, agent, certificates);
  worker_0.finish (job_submitted_0.wait());

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

  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  fhg::util::thread::event<std::string> job_submitted_0;
  utils::fake_drts_worker_waiting_for_finished_ack worker_0
    ([&job_submitted_0] (std::string j) { job_submitted_0.notify (j); }, agent, certificates);

  {
    fhg::util::thread::event<> job_submitted_1;

    const utils::fake_drts_worker_notifying_module_call_submission worker_1
      ( worker_id
      , [&job_submitted_1] (std::string) { job_submitted_1.notify(); }
      , agent
      , certificates
      );

    worker_0.finish_and_wait_for_ack (job_submitted_0.wait());

    job_submitted_1.wait();
  }

  const utils::fake_drts_worker_directly_finishing_jobs restarted_worker
    (worker_id, agent, certificates);
  worker_0.finish (job_submitted_0.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
