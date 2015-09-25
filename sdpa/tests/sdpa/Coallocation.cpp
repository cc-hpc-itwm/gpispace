#define BOOST_TEST_MODULE testCoallocation

#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_FIXTURE_TEST_CASE (testCoallocationWorkflow, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator, _logger);

  const utils::fake_drts_worker_directly_finishing_jobs worker_0 (agent);
  const utils::fake_drts_worker_directly_finishing_jobs worker_1 (agent);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (utils::net_with_one_child_requiring_workers (2).to_string(), orchestrator)
    , sdpa::status::FINISHED
    );
}

BOOST_FIXTURE_TEST_CASE (worker_shall_not_get_job_after_finishing_part_of_coallocation_job_while_other_workers_are_not_yet_done, setup_logging)
{
  //! \note issue #374

  // 0. setup environment orch -> agent.
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator, _logger);

  // 1. start worker 1
  fhg::util::thread::event<std::string> job_submitted_1;
  utils::fake_drts_worker_waiting_for_finished_ack worker_1
    ([&job_submitted_1] (std::string j) { job_submitted_1.notify (j); }, agent);

  // 2. submit workflow which generates tasks (2) that require 2 workers
  utils::client client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (2).to_string()));

  // 3. start worker 2
  fhg::util::thread::event<std::string> job_submitted_2;
  bool worker_2_shall_not_get_a_job (false);
  utils::fake_drts_worker_waiting_for_finished_ack worker_2
    ([&job_submitted_2, &worker_2_shall_not_get_a_job] (std::string j)
    {
      // 7. another job is submitted to worker 2 and worker 3
      BOOST_REQUIRE (!worker_2_shall_not_get_a_job);
      job_submitted_2.notify (j);
    }
    , agent
    );

  // 4. the job is correctly submitted to worker 1 and worker 2
  const std::string job_name (job_submitted_1.wait());
  BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());

  // 5. worker 2 is a slave and returns immediately
  worker_2_shall_not_get_a_job = true;
  worker_2.finish_and_wait_for_ack (job_name);

  {
    // 6. start worker 3
    utils::fake_drts_worker_notifying_module_call_submission worker_3
      ([] (std::string)
      {
        // 7. another job is submitted to worker 2 and worker 3
        BOOST_FAIL ("worker_3 shall never get a job: all other workers are busy");
      }
      , agent
      );

    //! \note Race and sleep! This is ugly, but needed to provoke a
    //! bad scheduling to worker_2 and worker_3. We sadly have no way
    //! to wait for the next scheduling loop without modifying the
    //! agent itself. The timeout is large enough that a loop should
    //! happen though: Scheduling should be triggered when worker_3
    //! registered.
    //! may be equivalent to agent._.request_scheduling();
    boost::this_thread::sleep (boost::posix_time::seconds (5));
  }

  // x. cleanup: terminate jobs.

  //! \note Race! We can't set the flag afterwards, though, as
  //! scheduling may already have happened.
  worker_2_shall_not_get_a_job = false;
  worker_1.finish_and_wait_for_ack (job_name);

  {
    const std::string _job_name (job_submitted_1.wait());
    BOOST_REQUIRE_EQUAL (_job_name, job_submitted_2.wait());

    worker_1.finish_and_wait_for_ack (_job_name);
    worker_2.finish_and_wait_for_ack (_job_name);
  }

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_FIXTURE_TEST_CASE (agent_is_scheduling_two_jobs_in_parallel_if_workers_are_available, setup_logging)
{
  //! \note related to issue #374

  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator, _logger);

  fhg::util::thread::event<std::string> job_submitted_1;
  utils::fake_drts_worker_waiting_for_finished_ack worker_1
    ([&job_submitted_1] (std::string j) { job_submitted_1.notify (j); }, agent);

  utils::client client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (2).to_string()));

  fhg::util::thread::event<std::string> job_submitted_2;
  bool worker_2_shall_not_get_a_job (false);
  utils::fake_drts_worker_waiting_for_finished_ack worker_2
    ([&job_submitted_2, &worker_2_shall_not_get_a_job] (std::string j)
    {
      BOOST_REQUIRE (!worker_2_shall_not_get_a_job);
      job_submitted_2.notify (j);
    }
    , agent
    );

  const std::string job_name (job_submitted_1.wait());
  BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());

  worker_2_shall_not_get_a_job = true;
  worker_2.finish_and_wait_for_ack (job_name);

  {
    //! \note starting _two_ workers, which get the second child job
    fhg::util::thread::event<std::string> job_submitted_3;
    utils::fake_drts_worker_waiting_for_finished_ack worker_3
      ([&job_submitted_3] (std::string j) { job_submitted_3.notify (j); }, agent);

    fhg::util::thread::event<std::string> job_submitted_4;
    utils::fake_drts_worker_waiting_for_finished_ack worker_4
      ([&job_submitted_4] (std::string j) { job_submitted_4.notify (j); }, agent);

    const std::string _job_name (job_submitted_3.wait());
    BOOST_REQUIRE_EQUAL (_job_name, job_submitted_4.wait());

    worker_3.finish_and_wait_for_ack (_job_name);
    worker_4.finish_and_wait_for_ack (_job_name);
  }

  //! \note less cleanup: second child job is executed by worker_3 and worker_4
  worker_1.finish_and_wait_for_ack (job_name);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_FIXTURE_TEST_CASE (worker_shall_not_get_job_after_finishing_and_another_worker_disappearing_while_not_all_workers_terminated, setup_logging)
{
  //! \note related to issue #374

  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator, _logger);

  utils::client client (orchestrator);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (3).to_string()));

  fhg::util::thread::event<std::string> job_submitted_1;
  fhg::util::thread::event<std::string> cancel_requested_1;
  bool worker_1_shall_not_get_a_job (false);
  utils::fake_drts_worker_notifying_cancel worker_1
    ([&job_submitted_1, &worker_1_shall_not_get_a_job] (std::string j)
    {
      BOOST_REQUIRE (!worker_1_shall_not_get_a_job);
      job_submitted_1.notify (j);
    }
    , [&cancel_requested_1] (std::string j) { cancel_requested_1.notify (j); }
    , agent
    );

  fhg::util::thread::event<std::string> job_submitted_2;
  fhg::util::thread::event<std::string> cancel_requested_2;
  utils::fake_drts_worker_notifying_cancel worker_2
    ( [&job_submitted_2] (std::string j) { job_submitted_2.notify (j); }
    , [&cancel_requested_2] (std::string j) { cancel_requested_2.notify (j); }
    , agent
    );

  {
    fhg::util::thread::event<std::string> job_submitted_3;

    const utils::fake_drts_worker_notifying_module_call_submission worker_3
      ( [&job_submitted_3] (std::string j) { job_submitted_3.notify (j); }
      , agent
      );

    std::string job_name (job_submitted_1.wait());
    BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());
    BOOST_REQUIRE_EQUAL (job_name, job_submitted_3.wait());
  }

  worker_1_shall_not_get_a_job = true;
  const std::string canceled_job_1 (cancel_requested_1.wait());
  worker_2.canceled (cancel_requested_2.wait());

  {
    utils::fake_drts_worker_notifying_module_call_submission worker_4
      ([] (std::string)
      {
        BOOST_FAIL ("worker_4 shall never get a job: worker 1 is still canceling");
      }
      , agent
      );
    utils::fake_drts_worker_notifying_module_call_submission worker_5
      ([] (std::string)
      {
        BOOST_FAIL ("worker_5 shall never get a job: workers 1 is still canceling");
      }
      , agent
      );

    //! \note Race and sleep! This is ugly, but needed to provoke a
    //! bad scheduling to worker_1, worker_4 and worker_5. We sadly
    //! have no way to wait for the next scheduling loop without
    //! modifying the agent itself. The timeout is large enough that a
    //! loop should happen though: Scheduling should be triggered when
    //! worker_4 and worker_5 are registered.
    //! may be equivalent to agent._.request_scheduling();
    boost::this_thread::sleep (boost::posix_time::seconds (5));
  }

  worker_1.canceled (canceled_job_1);
  //! \note Potential race?: agent thread may happen before this
  //! thread continuing and setting flag
  worker_1_shall_not_get_a_job = false;

  //! \note cleanup of both jobs
  {
    fhg::util::thread::event<std::string> job_submitted_3;

    utils::fake_drts_worker_waiting_for_finished_ack worker_3
      ( [&job_submitted_3] (std::string j) { job_submitted_3.notify (j); }
      , agent
      );

    {
      std::string job_name (job_submitted_1.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_3.wait());

      worker_1.finish (job_name);
      worker_2.finish (job_name);
      worker_3.finish_and_wait_for_ack (job_name);
    }
    {
      std::string job_name (job_submitted_1.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_3.wait());

      worker_1.finish (job_name);
      worker_2.finish (job_name);
      worker_3.finish_and_wait_for_ack (job_name);
    }
  }

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
