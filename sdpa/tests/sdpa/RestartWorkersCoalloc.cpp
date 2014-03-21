#define BOOST_TEST_MODULE restart_worker_with_coallocation_workflow

#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <sdpa/events/CancelJobEvent.hpp>

BOOST_GLOBAL_FIXTURE (setup_logging)

namespace
{
  class fake_drts_worker_waiting_for_cancel
    : public utils::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_waiting_for_cancel
        ( std::function<void (std::string)> announce_job
        , const utils::agent& master_agent
        )
      : utils::fake_drts_worker_notifying_module_call_submission
        (announce_job, master_agent)
      , _pending_cancel_requests (0)
    {}
    ~fake_drts_worker_waiting_for_cancel()
    {
      BOOST_REQUIRE_EQUAL (_pending_cancel_requests, 0);
    }

    void handleCancelJobEvent
      (const sdpa::events::CancelJobEvent* pEvt)
    {
      _network.perform
        ( sdpa::events::SDPAEvent::Ptr
          ( new sdpa::events::CancelJobAckEvent
            (_name, pEvt->from(), pEvt->job_id())
          )
        );

      boost::mutex::scoped_lock const _ (_mtx_cancel);
      ++_pending_cancel_requests;
      _cond_cancel.notify_one();
    }

    void wait_for_cancel()
    {
      boost::mutex::scoped_lock lock (_mtx_cancel);
      _cond_cancel.wait (lock);
      --_pending_cancel_requests;
    }

  private:
    boost::mutex _mtx_cancel;
    boost::condition_variable_any _cond_cancel;
    std::size_t _pending_cancel_requests;
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
  fake_drts_worker_waiting_for_cancel worker_0
    (boost::bind (&fhg::util::thread::event<std::string>::notify, &job_submitted_0, _1), agent);

  {
    fhg::util::thread::event<> job_submitted_1;

    const utils::fake_drts_worker_notifying_module_call_submission worker_1
      ( worker_id
      , boost::bind (&fhg::util::thread::event<>::notify, &job_submitted_1)
      , agent
      );

    job_submitted_0.wait();
    job_submitted_1.wait();
  }

  worker_0.wait_for_cancel();

  const utils::fake_drts_worker_directly_finishing_jobs restarted_worker
    (worker_id, agent);
  worker_0.finish (job_submitted_0.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

namespace
{
  class fake_drts_worker_waiting_for_finished_ack
    : public utils::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_waiting_for_finished_ack
        ( std::function<void (std::string)> announce_job
        , const utils::agent& master_agent
        )
      : utils::fake_drts_worker_notifying_module_call_submission
        (announce_job, master_agent)
    {}

    void handleJobFinishedAckEvent
      (const sdpa::events::JobFinishedAckEvent* e)
    {
      _finished_ack.notify (e->job_id());
    }

    void finish_and_wait_for_ack (std::string name)
    {
      const std::string expected_id (_jobs.at (name)._id);

      finish (name);

      BOOST_REQUIRE_EQUAL (_finished_ack.wait(), expected_id);
    }

  private:
    fhg::util::thread::event<std::string> _finished_ack;
  };
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
  fake_drts_worker_waiting_for_finished_ack worker_0
    (boost::bind (&fhg::util::thread::event<std::string>::notify, &job_submitted_0, _1), agent);

  {
    fhg::util::thread::event<> job_submitted_1;

    const utils::fake_drts_worker_notifying_module_call_submission worker_1
      ( worker_id
      , boost::bind (&fhg::util::thread::event<>::notify, &job_submitted_1)
      , agent
      );

    worker_0.finish_and_wait_for_ack (job_submitted_0.wait());

    job_submitted_1.wait();
  }

  const utils::fake_drts_worker_directly_finishing_jobs restarted_worker
    (worker_id, agent);
  worker_0.finish (job_submitted_0.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
