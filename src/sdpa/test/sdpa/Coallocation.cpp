// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <test/certificates_data.hpp>

#include <fhg/util/thread/event.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

BOOST_DATA_TEST_CASE (coallocation_workflow, certificates_data, certificates)
{
  const utils::agent agent (certificates);

  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2)));

  fhg::util::thread::event<std::string> job_submitted_0;
  utils::fake_drts_worker_waiting_for_finished_ack worker_0
    ( [&job_submitted_0] (std::string s) { job_submitted_0.notify (s); }
    , agent
    , certificates
    );

  fhg::util::thread::event<std::string> job_submitted_1;
  utils::fake_drts_worker_waiting_for_finished_ack worker_1
    ( [&job_submitted_1] (std::string s) { job_submitted_1.notify (s); }
    , agent
    , certificates
    );

  worker_0.finish_and_wait_for_ack (job_submitted_0.wait());
  worker_1.finish_and_wait_for_ack (job_submitted_1.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_DATA_TEST_CASE
  ( worker_shall_not_get_job_after_finishing_part_of_coallocation_job_while_other_workers_are_not_yet_done
  , certificates_data
  , certificates
  )
{
  //! \note issue #374

  // 0. setup agent
  const utils::agent agent (certificates);

  // 1. start worker 1
  fhg::util::thread::event<std::string> job_submitted_1;
  utils::fake_drts_worker_waiting_for_finished_ack worker_1
    ([&job_submitted_1] (std::string j) { job_submitted_1.notify (j); }, agent, certificates);

  // 2. submit workflow which generates tasks (2) that require 2 workers
  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (2)));

  // 3. start worker 2
  fhg::util::thread::event<std::string> job_submitted_2;
  std::atomic<bool> worker_2_shall_not_get_a_job (false);
  std::atomic<bool> worker_2_got_a_job_while_forbidden (false);
  utils::fake_drts_worker_waiting_for_finished_ack worker_2
    ( [&] (std::string j)
      {
        // \note Notifying after the check to avoid a race between
        // setting the forbidden-flag and checking!
        // bad: 7. a job is submitted while coallocated worker is
        // still active
        worker_2_got_a_job_while_forbidden
          = worker_2_got_a_job_while_forbidden || worker_2_shall_not_get_a_job;
        // 4.1. worker 2 gets the job
        job_submitted_2.notify (j);
      }
    , agent
    , certificates
    );

  // 4. the job is correctly submitted to worker 1 and worker 2
  const std::string job_name (job_submitted_1.wait());
  BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());

  // 5. worker 2 is a child and returns immediately
  worker_2_shall_not_get_a_job = true;
  worker_2.finish_and_wait_for_ack (job_name);

  std::atomic<bool> worker_3_got_a_job (false);

  {
    // 6. start worker 3
    utils::fake_drts_worker_notifying_module_call_submission worker_3
      ( [&] (std::string)
        {
          // bad: 7. another job is submitted to worker 2 and worker 3
          worker_3_got_a_job = true;
        }
      , agent
      , certificates
      );

    //! \note Race and sleep! This is ugly, but needed to provoke a
    //! bad scheduling to worker_2 and worker_3. We sadly have no way
    //! to wait for the next scheduling loop without modifying the
    //! agent itself. The timeout is large enough that a loop should
    //! happen though: Scheduling should be triggered when worker_3
    //! registered.
    //! may be equivalent to agent._.request_scheduling();
    std::this_thread::sleep_for (std::chrono::seconds (2));
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

  BOOST_REQUIRE_MESSAGE
    ( !worker_3_got_a_job
    , "worker_3 shall never get a job: all other workers are busy"
    );
  BOOST_REQUIRE_MESSAGE
    ( !worker_2_got_a_job_while_forbidden
    , "worker_2 shall not get a job: coallocated worker still busy"
    );
}

BOOST_DATA_TEST_CASE
  ( agent_is_scheduling_two_jobs_in_parallel_if_workers_are_available
  , certificates_data
  , certificates
  )
{
  //! \note related to issue #374

  const utils::agent agent (certificates);

  fhg::util::thread::event<std::string> job_submitted_1;
  utils::fake_drts_worker_waiting_for_finished_ack worker_1
    ([&job_submitted_1] (std::string j) { job_submitted_1.notify (j); }, agent, certificates);

  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (2)));

  fhg::util::thread::event<std::string> job_submitted_2;
  std::atomic<bool> worker_2_shall_not_get_a_job (false);
  std::atomic<bool> worker_2_got_a_job_while_forbidden (false);
  utils::fake_drts_worker_waiting_for_finished_ack worker_2
    ( [&] (std::string j)
      {
        // \note Notifying after the check to avoid a race between
        // setting the forbidden-flag and checking!
        worker_2_got_a_job_while_forbidden
          = worker_2_got_a_job_while_forbidden || worker_2_shall_not_get_a_job;
        job_submitted_2.notify (j);
      }
    , agent
    , certificates
    );

  const std::string job_name (job_submitted_1.wait());
  BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());

  worker_2_shall_not_get_a_job = true;
  worker_2.finish_and_wait_for_ack (job_name);

  {
    //! \note starting _two_ workers, which get the second child job
    fhg::util::thread::event<std::string> job_submitted_3;
    utils::fake_drts_worker_waiting_for_finished_ack worker_3
      ([&job_submitted_3] (std::string j) { job_submitted_3.notify (j); }, agent, certificates);

    fhg::util::thread::event<std::string> job_submitted_4;
    utils::fake_drts_worker_waiting_for_finished_ack worker_4
      ([&job_submitted_4] (std::string j) { job_submitted_4.notify (j); }, agent, certificates);

    const std::string _job_name (job_submitted_3.wait());
    BOOST_REQUIRE_EQUAL (_job_name, job_submitted_4.wait());

    worker_3.finish_and_wait_for_ack (_job_name);
    worker_4.finish_and_wait_for_ack (_job_name);
  }

  //! \note less cleanup: second child job is executed by worker_3 and worker_4
  worker_1.finish_and_wait_for_ack (job_name);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);

  BOOST_REQUIRE_MESSAGE
    ( !worker_2_got_a_job_while_forbidden
    , "worker_2 shall not get a second job: two other workers are available"
    );
}

BOOST_DATA_TEST_CASE
  ( worker_shall_not_get_job_after_finishing_and_another_worker_disappearing_while_not_all_workers_terminated
  , certificates_data
  , certificates
  )
{
  //! \note related to issue #374

  const utils::agent agent (certificates);

  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (3)));

  fhg::util::thread::event<std::string> job_submitted_1;
  fhg::util::thread::event<std::string> cancel_requested_1;
  std::atomic<bool> worker_1_shall_not_get_a_job (false);
  std::atomic<bool> worker_1_got_a_job_while_forbidden (false);
  utils::fake_drts_worker_notifying_cancel worker_1
    ( [&] (std::string j)
      {
        // \note Notifying after the check to avoid a race between
        // setting the forbidden-flag and checking!
        worker_1_got_a_job_while_forbidden
          = worker_1_got_a_job_while_forbidden || worker_1_shall_not_get_a_job;
        job_submitted_1.notify (j);
    }
    , [&cancel_requested_1] (std::string j) { cancel_requested_1.notify (j); }
    , agent
    , certificates
    );

  fhg::util::thread::event<std::string> job_submitted_2;
  fhg::util::thread::event<std::string> cancel_requested_2;
  utils::fake_drts_worker_notifying_cancel worker_2
    ( [&job_submitted_2] (std::string j) { job_submitted_2.notify (j); }
    , [&cancel_requested_2] (std::string j) { cancel_requested_2.notify (j); }
    , agent
    , certificates
    );

  {
    fhg::util::thread::event<std::string> job_submitted_3;

    const utils::fake_drts_worker_notifying_module_call_submission worker_3
      ( [&job_submitted_3] (std::string j) { job_submitted_3.notify (j); }
      , agent
      , certificates
      );

    std::string job_name (job_submitted_1.wait());
    BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());
    BOOST_REQUIRE_EQUAL (job_name, job_submitted_3.wait());
  }

  worker_1_shall_not_get_a_job = true;
  const std::string canceled_job_1 (cancel_requested_1.wait());
  worker_2.canceled (cancel_requested_2.wait());

  std::atomic<bool> worker_4_or_5_got_a_job (false);

  {
    utils::fake_drts_worker_notifying_module_call_submission worker_4
      ( [&] (std::string)
        {
          worker_4_or_5_got_a_job = true;
        }
      , agent
      , certificates
      );
    utils::fake_drts_worker_notifying_module_call_submission worker_5
      ( [&] (std::string)
        {
          worker_4_or_5_got_a_job = true;
        }
      , agent
      , certificates
      );

    //! \note Race and sleep! This is ugly, but needed to provoke a
    //! bad scheduling to worker_1, worker_4 and worker_5. We sadly
    //! have no way to wait for the next scheduling loop without
    //! modifying the agent itself. The timeout is large enough that a
    //! loop should happen though: Scheduling should be triggered when
    //! worker_4 and worker_5 are registered.
    //! may be equivalent to agent._.request_scheduling();
    std::this_thread::sleep_for (std::chrono::seconds (5));
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
      , certificates
      );

    {
      std::string job_name (job_submitted_1.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_3.wait());

      worker_1.finish_and_wait_for_ack (job_name);
      worker_2.finish_and_wait_for_ack (job_name);
      worker_3.finish_and_wait_for_ack (job_name);
    }
    {
      std::string job_name (job_submitted_1.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_3.wait());

      worker_1.finish_and_wait_for_ack (job_name);
      worker_2.finish_and_wait_for_ack (job_name);
      worker_3.finish_and_wait_for_ack (job_name);
    }
  }

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);

  BOOST_REQUIRE_MESSAGE
    ( !worker_1_got_a_job_while_forbidden
    , "worker_1 shall not get a job: has not yet cancel-acked"
    );

  BOOST_REQUIRE_MESSAGE
    ( !worker_4_or_5_got_a_job
    , "worker_4 shall never get a job: worker 1 is still canceling"
    );
}

BOOST_DATA_TEST_CASE
  ( coallocated_tasks_are_canceled_when_one_worker_having_a_sibling_task_dies
  , certificates_data
  , certificates
  )
{
  //! \note related to issue #822

  const utils::agent agent (certificates);

  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (3)));

  fhg::util::thread::event<std::string> job_submitted_1;
  fhg::util::thread::event<std::string> cancel_requested_1;
  utils::fake_drts_worker_notifying_cancel worker_1
    ( [&] (std::string j) { job_submitted_1.notify (j); }
    , [&cancel_requested_1] (std::string j) { cancel_requested_1.notify (j); }
    , agent
    , certificates
    );

  fhg::util::thread::event<std::string> job_submitted_2;
  fhg::util::thread::event<std::string> cancel_requested_2;
  utils::fake_drts_worker_notifying_cancel worker_2
    ( [&job_submitted_2] (std::string j) { job_submitted_2.notify (j); }
    , [&cancel_requested_2] (std::string j) { cancel_requested_2.notify (j); }
    , agent
    , certificates
    );

  {
    fhg::util::thread::event<std::string> job_submitted_3;

    const utils::fake_drts_worker_notifying_module_call_submission worker_3
      ( [&job_submitted_3] (std::string j) { job_submitted_3.notify (j); }
      , agent
      , certificates
      );

    std::string job_name (job_submitted_1.wait());
    BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());
    BOOST_REQUIRE_EQUAL (job_name, job_submitted_3.wait());
  }

  const std::string canceled_job_1 (cancel_requested_1.wait());
  worker_2.canceled (cancel_requested_2.wait());
  worker_1.canceled (canceled_job_1);

  {
    fhg::util::thread::event<std::string> job_submitted_3;

    utils::fake_drts_worker_waiting_for_finished_ack worker_3
      ( [&job_submitted_3] (std::string j) { job_submitted_3.notify (j); }
      , agent
      , certificates
      );

    {
      std::string job_name (job_submitted_1.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_3.wait());

      worker_1.finish_and_wait_for_ack (job_name);
      worker_2.finish_and_wait_for_ack (job_name);
      worker_3.finish_and_wait_for_ack (job_name);
    }
    {
      std::string job_name (job_submitted_1.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());
      BOOST_REQUIRE_EQUAL (job_name, job_submitted_3.wait());

      worker_1.finish_and_wait_for_ack (job_name);
      worker_2.finish_and_wait_for_ack (job_name);
      worker_3.finish_and_wait_for_ack (job_name);
    }
  }

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_DATA_TEST_CASE
  ( reschedule_happens_even_though_all_others_were_success
  , certificates_data
  , certificates
  )
{
  const utils::agent agent (certificates);

  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2)));

  fhg::util::thread::event<std::string> job_submitted_1;
  auto worker_1
    ( std::make_unique<utils::fake_drts_worker_notifying_module_call_submission>
        ( [&job_submitted_1] (std::string j) { job_submitted_1.notify (j); }
        , agent
        , certificates
        )
    );

  fhg::util::thread::event<std::string> job_submitted_2;
  fhg::util::thread::event<std::string> cancel_requested_2;
  utils::fake_drts_worker_notifying_cancel_but_never_replying worker_2
    ( [&job_submitted_2] (std::string j) { job_submitted_2.notify (j); }
    , [&cancel_requested_2] (std::string j) { cancel_requested_2.notify (j); }
    , agent
    , certificates
    );

  std::string cancel_id;
  {
    sdpa::job_id_t const submitted (job_submitted_1.wait());
    BOOST_REQUIRE_EQUAL (submitted, job_submitted_2.wait());
    worker_1.reset();
    cancel_id = cancel_requested_2.wait();
    worker_2.finish_and_wait_for_ack (submitted);
  }

  fhg::util::thread::event<std::string> job_submitted_3;
  utils::fake_drts_worker_waiting_for_finished_ack worker_3
    ( [&job_submitted_3] (std::string j) { job_submitted_3.notify (j); }
    , agent
    , certificates
    );

  {
    sdpa::job_id_t const submitted (job_submitted_2.wait());
    BOOST_REQUIRE_EQUAL (submitted, job_submitted_3.wait());

    worker_2.finish_and_wait_for_ack (submitted);
    worker_3.finish_and_wait_for_ack (submitted);
  }

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_DATA_TEST_CASE
  ( worker_submits_result_and_dies_the_still_running_coallocated_jobs_are_canceled
  , certificates_data
  , certificates
  )
{
  const utils::agent agent (certificates);

  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (3)));

  fhg::util::thread::event<std::string> job_submitted_1;
  fhg::util::thread::event<std::string> cancel_requested_1;

  utils::fake_drts_worker_notifying_cancel worker_1
    ( [&] (std::string j) { job_submitted_1.notify (j); }
    , [&cancel_requested_1] (std::string j) { cancel_requested_1.notify (j); }
    , agent
    , certificates
    );

  fhg::util::thread::event<std::string> job_submitted_2;
  fhg::util::thread::event<std::string> cancel_requested_2;
  utils::fake_drts_worker_notifying_cancel worker_2
    ( [&job_submitted_2] (std::string j) { job_submitted_2.notify (j); }
    , [&cancel_requested_2] (std::string j) { cancel_requested_2.notify (j); }
    , agent
    , certificates
    );

  {
    fhg::util::thread::event<std::string> job_submitted_3;

    utils::fake_drts_worker_waiting_for_finished_ack worker_3
      ( [&job_submitted_3] (std::string j) { job_submitted_3.notify (j); }
      , agent
      , certificates
      );

    std::string job_name (job_submitted_1.wait());
    BOOST_REQUIRE_EQUAL (job_name, job_submitted_2.wait());
    BOOST_REQUIRE_EQUAL (job_name, job_submitted_3.wait());

    // worker 3 submits the result and dies
    worker_3.finish_and_wait_for_ack (job_name);
  }

  // it is expected that the other 2 running tasks are canceled
  auto const canceled_job_1 (cancel_requested_1.wait());
  worker_1.canceled (canceled_job_1);
  auto const canceled_job_2 (cancel_requested_2.wait());
  BOOST_REQUIRE_EQUAL (canceled_job_2, canceled_job_1);
  worker_2.canceled (canceled_job_2);
}
