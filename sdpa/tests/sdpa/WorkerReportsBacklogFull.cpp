#define BOOST_TEST_MODULE WORKER_REPORTS_BACKLOG_FULL

#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/BacklogNoLongerFullEvent.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>

BOOST_GLOBAL_FIXTURE (setup_logging)

namespace
{
  class fake_drts_worker_reporting_backlog_full
    : public utils::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_reporting_backlog_full
      ( std::function<void (std::string)> announce_job
      , utils::agent const& master
      )
      : utils::fake_drts_worker_notifying_module_call_submission (announce_job, master)
    {}

    virtual void handleSubmitJobEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobEvent* e) override
    {
      const std::string name
        (we::type::activity_t (e->description()).transition().name());

      add_job (name, *e->job_id(), source);
      announce_job (name);
    }

    void report_backlog_full (std::string name)
    {
      const job_t job (_jobs.at (name));
      _jobs.erase (name);

      _network.perform
        ( job._owner
        , sdpa::events::SDPAEvent::Ptr
            (new sdpa::events::ErrorEvent ( sdpa::events::ErrorEvent::SDPA_EBACKLOGFULL
                                          , "Cannot accept this job, my backlog is full!"
                                          , job._id
                                          )
             )
        );
    }

    void report_can_take_jobs()
    {
      _network.perform
        ( _master.get()
        , sdpa::events::SDPAEvent::Ptr (new sdpa::events::BacklogNoLongerFullEvent())
        );
    }

    void acknowledge_and_finish (std::string name)
    {
      const job_t job (_jobs.at (name));

      _network.perform
        ( job._owner
        , sdpa::events::SDPAEvent::Ptr
            (new sdpa::events::SubmitJobAckEvent (job._id))
        );

      _network.perform
        ( job._owner
        , sdpa::events::SDPAEvent::Ptr
            ( new sdpa::events::JobFinishedEvent
                (job._id, we::type::activity_t().to_string())
            )
        );

      _jobs.erase (name);
    }
  };
}

BOOST_AUTO_TEST_CASE (one_worker_reports_backlog_full_the_other_two_receive_cancellation_requests)
{
  const utils::orchestrator orchestrator;
  const utils::agent agent (orchestrator);

  utils::client client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (3).to_string()));

  fhg::util::thread::event<std::string> job_submitted_1;

  fake_drts_worker_reporting_backlog_full worker_1
    ([&job_submitted_1] (std::string j){job_submitted_1.notify (j);}
    , agent
    );

  fhg::util::thread::event<std::string> job_submitted_2;
  fhg::util::thread::event<std::string> cancel_requested_2;
  utils::fake_drts_worker_notifying_cancel worker_2
    ( [&job_submitted_2] (std::string j) { job_submitted_2.notify (j);}
    , [&cancel_requested_2] (std::string j) { cancel_requested_2.notify (j); }
    , agent
    );

  fhg::util::thread::event<std::string> job_submitted_3;
  fhg::util::thread::event<std::string> cancel_requested_3;
  utils::fake_drts_worker_notifying_cancel worker_3
    ( [&job_submitted_3] (std::string j) { job_submitted_3.notify (j);}
    , [&cancel_requested_3] (std::string j) { cancel_requested_3.notify (j); }
    , agent
    );

  const std::string job_name (job_submitted_1.wait());
  worker_1.report_backlog_full (job_name);

  std::string const job_name_2 (job_submitted_2.wait());
  std::string const job_name_3 (job_submitted_3.wait());

  sdpa::job_id_t const job_id_2 (worker_2.job_id (job_name_2));
  sdpa::job_id_t const job_id_3 (worker_3.job_id (job_name_3));

  BOOST_REQUIRE_EQUAL (cancel_requested_2.wait(), job_id_2);
  BOOST_REQUIRE_EQUAL (cancel_requested_3.wait(), job_id_3);

  worker_2.canceled (job_id_2);
  worker_3.canceled (job_id_3);
}

BOOST_AUTO_TEST_CASE (one_worker_reports_backlog_full_the_2_siblings_are_cancelled_the_job_is_rescheduled)
{
  const utils::orchestrator orchestrator;
  const utils::agent agent (orchestrator);

  utils::client client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (3).to_string()));

  fhg::util::thread::event<std::string> job_submitted_1;

  fake_drts_worker_reporting_backlog_full worker_1
    ([&job_submitted_1] (std::string j){job_submitted_1.notify (j);}
    , agent
    );

  fhg::util::thread::event<std::string> job_submitted_2;
  fhg::util::thread::event<std::string> cancel_requested_2;
  utils::fake_drts_worker_notifying_cancel worker_2
    ( [&job_submitted_2] (std::string j) { job_submitted_2.notify (j);}
    , [&cancel_requested_2] (std::string j) { cancel_requested_2.notify (j); }
    , agent
    );

  fhg::util::thread::event<std::string> job_submitted_3;
  fhg::util::thread::event<std::string> cancel_requested_3;
  utils::fake_drts_worker_notifying_cancel worker_3
    ( [&job_submitted_3] (std::string j) { job_submitted_3.notify (j);}
    , [&cancel_requested_3] (std::string j) { cancel_requested_3.notify (j); }
    , agent
    );

  const std::string job_name_1 (job_submitted_1.wait());
  job_submitted_2.wait();
  job_submitted_3.wait();

  const utils::fake_drts_worker_directly_finishing_jobs worker_4 (agent);

  worker_1.report_backlog_full (job_name_1);
  worker_2.canceled (cancel_requested_2.wait());
  worker_3.canceled (cancel_requested_3.wait());

  const std::string job_name_2 (job_submitted_2.wait());
  const std::string job_name_3 (job_submitted_3.wait());

  BOOST_REQUIRE_EQUAL (job_name_2, job_name_3);

  worker_2.finish_and_wait_for_ack (job_name_2);
  worker_3.finish_and_wait_for_ack (job_name_3);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_AUTO_TEST_CASE (one_worker_reports_backlog_full_the_still_running_sibling_is_cancelled_the_job_is_rescheduled)
{
  const utils::orchestrator orchestrator;
  const utils::agent agent (orchestrator);

  utils::client client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (3).to_string()));

  fhg::util::thread::event<std::string> job_submitted_1;
  utils::fake_drts_worker_waiting_for_finished_ack worker_1
      ([&job_submitted_1] (std::string j) { job_submitted_1.notify (j); }, agent);

  fhg::util::thread::event<std::string> job_submitted_2;
  fake_drts_worker_reporting_backlog_full worker_2
    ([&job_submitted_2] (std::string j){job_submitted_2.notify (j);}
    , agent
    );

  fhg::util::thread::event<std::string> job_submitted_3;
  fhg::util::thread::event<std::string> cancel_requested_3;
  utils::fake_drts_worker_notifying_cancel worker_3
    ( [&job_submitted_3] (std::string j) { job_submitted_3.notify (j);}
    , [&cancel_requested_3] (std::string j) { cancel_requested_3.notify (j); }
    , agent
    );

  const std::string job_name (job_submitted_1.wait());
  job_submitted_2.wait();
  job_submitted_3.wait();

  fhg::util::thread::event<std::string> job_submitted_4;
  utils::fake_drts_worker_waiting_for_finished_ack worker_4
    ([&job_submitted_4] (std::string j) { job_submitted_4.notify (j); }, agent);

  worker_1.finish_and_wait_for_ack (job_name);
  worker_2.report_backlog_full (job_name);
  worker_3.canceled (cancel_requested_3.wait());

  std::string job_name_1 (job_submitted_1.wait());
  std::string job_name_3 (job_submitted_3.wait());
  std::string job_name_4 (job_submitted_4.wait());

  BOOST_REQUIRE_EQUAL (job_name_1, job_name_3);
  BOOST_REQUIRE_EQUAL (job_name_1, job_name_4);

  worker_1.finish_and_wait_for_ack (job_name_1);
  worker_3.finish_and_wait_for_ack (job_name_3);
  worker_4.finish_and_wait_for_ack (job_name_4);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_AUTO_TEST_CASE
  (one_worker_reports_backlog_full_the_second_activity_terminates_the_first_activity_is_rescheduled)
{
  const utils::orchestrator orchestrator;
  const utils::agent agent (orchestrator);

  utils::client client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (2).to_string()));

  fhg::util::thread::event<std::string> job_submitted_1;

  fake_drts_worker_reporting_backlog_full worker_1
    ([&job_submitted_1] (std::string j){job_submitted_1.notify (j);}
    , agent
    );

  fhg::util::thread::event<std::string> job_submitted_2;
  fhg::util::thread::event<std::string> cancel_requested_2;
  utils::fake_drts_worker_notifying_cancel worker_2
    ( [&job_submitted_2] (std::string j) { job_submitted_2.notify (j);}
    , [&cancel_requested_2] (std::string j) { cancel_requested_2.notify (j); }
    , agent
    );

  const std::string job_name_1 (job_submitted_1.wait());
  BOOST_REQUIRE_EQUAL (job_name_1, job_submitted_2.wait());

  worker_1.report_backlog_full (job_name_1);
  worker_2.canceled (cancel_requested_2.wait());

  const std::string job_name_2 (job_submitted_1.wait());
  BOOST_REQUIRE_EQUAL (job_name_2, job_submitted_2.wait());
  BOOST_REQUIRE (job_name_2 != job_name_1);

  worker_1.acknowledge_and_finish (job_name_2);
  worker_2.finish_and_wait_for_ack (job_name_2);

  worker_1.report_can_take_jobs();

  const std::string job_name_3 (job_submitted_1.wait());
  BOOST_REQUIRE_EQUAL (job_name_3, job_submitted_2.wait());
  BOOST_REQUIRE_EQUAL (job_name_3, job_name_1);

  worker_1.acknowledge_and_finish (job_name_3);
  worker_2.finish_and_wait_for_ack (job_name_3);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}