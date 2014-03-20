#define BOOST_TEST_MODULE restart_worker_with_coallocation_workflow

#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <sdpa/events/CancelJobEvent.hpp>

BOOST_GLOBAL_FIXTURE (setup_logging)

namespace
{
  class fake_drts_worker_waiting_finished_ack_and_replying_cancel_ack
    : public utils::fake_drts_worker_notifying_module_call_submission
  {
  public:
      fake_drts_worker_waiting_finished_ack_and_replying_cancel_ack
        ( boost::function<void (std::string)> announce_job
        , const utils::agent& master_agent
        )
      : utils::fake_drts_worker_notifying_module_call_submission
        (announce_job, master_agent)
      , _n_cancel_requests(0)
    {}

    void handleCancelJobEvent
      (const sdpa::events::CancelJobEvent* pEvt)
    {
     _network.perform
            ( sdpa::events::SDPAEvent::Ptr
              (new sdpa::events::CancelJobAckEvent (_name, pEvt->from(), pEvt->job_id()))
            );

     _n_cancel_requests++;
    }

    void handleJobFinishedAckEvent
      (const sdpa::events::JobFinishedAckEvent*)
    {
      _cond_finished.notify_all();
    }

    void wait_finished_ack ()
    {
      boost::unique_lock<boost::mutex> lock (_mtx_finished);
      _cond_finished.wait(lock);
    }

    bool n_cancel_requests () { return _n_cancel_requests; }
  private:
    boost::mutex _mtx_finished;
    boost::condition_variable_any _cond_finished;
    int _n_cancel_requests;
  };
}

BOOST_AUTO_TEST_CASE (restart_workers_while_job_requiring_coallocation_is_running)
{
  const utils::kvs_server kvs_server;
  const utils::orchestrator orchestrator (kvs_server);
  const utils::agent agent (orchestrator);

  utils::client client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2).to_string()));

  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  fhg::util::thread::event<std::string> job_submitted_0;
  fake_drts_worker_waiting_finished_ack_and_replying_cancel_ack worker_0
    (boost::bind (&fhg::util::thread::event<std::string>::notify, &job_submitted_0, _1), agent);

  {
    fhg::util::thread::event<> job_submitted_1;

    const utils::fake_drts_worker_notifying_module_call_submission worker_1
      ( worker_id
      , boost::bind (&fhg::util::thread::event<>::notify, &job_submitted_1)
      , agent
      );

    std::string ignore;
    job_submitted_0.wait (ignore);
    job_submitted_1.wait();
  }

  const utils::fake_drts_worker_directly_finishing_jobs restarted_worker
    (worker_id, agent);
  std::string job_id_0;
  job_submitted_0.wait (job_id_0);
  worker_0.finish (job_id_0);

  BOOST_REQUIRE_EQUAL(worker_0.n_cancel_requests(), 1);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_AUTO_TEST_CASE (restart_workers_while_job_is_running_and_partial_result_is_missing)
{
  const utils::kvs_server kvs_server;
  const utils::orchestrator orchestrator (kvs_server);
  const utils::agent agent (orchestrator);

  utils::client client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2).to_string()));

  sdpa::worker_id_t const worker_id (utils::random_peer_name());

  fhg::util::thread::event<std::string> job_submitted_0;
  fake_drts_worker_waiting_finished_ack_and_replying_cancel_ack worker_0
    (boost::bind (&fhg::util::thread::event<std::string>::notify, &job_submitted_0, _1), agent);

  {
    fhg::util::thread::event<> job_submitted_1;

    const utils::fake_drts_worker_notifying_module_call_submission worker_1
      ( worker_id
      , boost::bind (&fhg::util::thread::event<>::notify, &job_submitted_1)
      , agent
      );

    std::string name_of_job_on_surviving_worker;
    job_submitted_0.wait (name_of_job_on_surviving_worker);
    worker_0.finish (name_of_job_on_surviving_worker);

    worker_0.wait_finished_ack ();

    job_submitted_1.wait();
  }

  const utils::fake_drts_worker_directly_finishing_jobs restarted_worker
    (worker_id, agent);
  std::string job_id_0;
  job_submitted_0.wait (job_id_0);
  worker_0.finish (job_id_0);

  BOOST_REQUIRE_EQUAL(worker_0.n_cancel_requests(), 0);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
