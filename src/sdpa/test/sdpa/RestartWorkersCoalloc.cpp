// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <testing/certificates_data.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <chrono>
#include <functional>
#include <future>
#include <string>

BOOST_DATA_TEST_CASE
  ( restart_workers_while_job_requiring_coallocation_is_running
  , certificates_data
  , certificates
  )
{
  const utils::agent agent (certificates);

  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2)));

  sdpa::worker_id_t worker_id;

  fhg::util::threadsafe_queue<std::string> job_submitted_0;
  std::promise<std::string> cancel_requested_0;
  utils::fake_drts_worker_notifying_cancel worker_0
    ( [&] (std::string j) { job_submitted_0.put (j); }
    , [&cancel_requested_0] (std::string j) { cancel_requested_0.set_value (j); }
    , agent
    , certificates
    );

  {
    std::promise<void> job_submitted_1;

    const utils::fake_drts_worker_waiting_for_finished_ack worker_1
      ( [&job_submitted_1] (std::string) { job_submitted_1.set_value(); }
      , agent
      , certificates
      );

    worker_id = worker_1.name();

    job_submitted_0.get();
    job_submitted_1.get_future().wait();
  }

  worker_0.canceled (cancel_requested_0.get_future().get());

  fhg::util::threadsafe_queue<std::string> job_submitted_to_restarted_worker;
  utils::fake_drts_worker_waiting_for_finished_ack restarted_worker
    ( utils::reused_component_name (worker_id)
    , [&job_submitted_to_restarted_worker] (std::string s)
      { job_submitted_to_restarted_worker.put (s); }
    , agent
    , certificates
    );

  worker_0.finish_and_wait_for_ack (job_submitted_0.get());
  restarted_worker.finish_and_wait_for_ack
    (job_submitted_to_restarted_worker.get());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

BOOST_DATA_TEST_CASE
  ( restart_workers_while_job_is_running_and_partial_result_is_missing
  , certificates_data
  , certificates
  )
{
  const utils::agent agent (certificates);

  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2)));

  sdpa::worker_id_t worker_id;

  fhg::util::threadsafe_queue<std::string> job_submitted_0;
  utils::fake_drts_worker_waiting_for_finished_ack worker_0
    ([&job_submitted_0] (std::string j) { job_submitted_0.put (j); }, agent, certificates);

  {
    std::promise<void> job_submitted_1;

    const utils::fake_drts_worker_waiting_for_finished_ack worker_1
      ( [&job_submitted_1] (std::string) { job_submitted_1.set_value(); }
      , agent
      , certificates
      );

    worker_id = worker_1.name();

    worker_0.finish_and_wait_for_ack (job_submitted_0.get());

    job_submitted_1.get_future().wait();
  }

  fhg::util::threadsafe_queue<std::string> job_submitted_to_restarted_worker;
  utils::fake_drts_worker_waiting_for_finished_ack restarted_worker
    ( utils::reused_component_name (worker_id)
    , [&job_submitted_to_restarted_worker] (std::string s)
      { job_submitted_to_restarted_worker.put (s); }
    , agent
    , certificates
    );

  worker_0.finish_and_wait_for_ack (job_submitted_0.get());
  restarted_worker.finish_and_wait_for_ack
    (job_submitted_to_restarted_worker.get());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}

namespace
{
  class fake_drts_worker_notifying_submission_and_cancel final
    : public utils::no_thread::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_notifying_submission_and_cancel
        ( std::function<void (std::string)> announce_job
        , std::function<void (std::string)> announce_cancel
        , utils::agent const& parent_agent
        , fhg::com::Certificates const& certificates
        )
      : utils::no_thread::fake_drts_worker_notifying_module_call_submission
          (announce_job, parent_agent, certificates)
      , _announce_cancel (announce_cancel)
      , _agent_address (parent_agent.host(), parent_agent.port())
    {}

    void handleCancelJobEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent* e) override
    {
      auto const job_id (e->job_id());
      if (source != _agent_address)
      {
        throw std::runtime_error ("received cancel request from unknown agent!");
      }

      _announce_cancel (job_id);
    }

    void canceled (std::string job_id)
    {
      _network.perform<sdpa::events::CancelJobAckEvent> (_agent_address, job_id);
    }

  private:
    std::function<void (std::string)> _announce_cancel;
    fhg::com::p2p::address_t _agent_address;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };
}

BOOST_DATA_TEST_CASE
  ( retry_job_a_specified_number_of_times
  , certificates_data
  , certificates
  )
{
  unsigned long const num_workers (2);
  unsigned long const maximum_number_of_retries (2);

  utils::agent const agent (certificates);

  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id
    ( client.submit_job
      ( utils::net_with_one_child_requiring_workers_and_num_retries
          (num_workers, maximum_number_of_retries)
      )
    );

  fhg::util::threadsafe_queue<std::string> job_submitted_0;
  fhg::util::threadsafe_queue<std::string> cancel_requested_0;

  fake_drts_worker_notifying_submission_and_cancel worker_0
    ( [&] (std::string j) { job_submitted_0.put (j); }
    , [&] (std::string j) { cancel_requested_0.put (j); }
    , agent
    , certificates
    );

  fhg::util::threadsafe_queue<std::string> job_submitted_1;
  std::unique_ptr<utils::fake_drts_worker_notifying_module_call_submission>  worker_1
    ( new utils::fake_drts_worker_notifying_module_call_submission
        ( [&] (std::string k) { job_submitted_1.put (k); }
        , agent
        , certificates
        )
    );

  for (unsigned long i {0}; i <= maximum_number_of_retries; ++i)
  {
    if (i > 0)
    {
      //if it's not the first submission, worker_0 waits for the
      //previous submission to be cancelled
      worker_0.canceled (cancel_requested_0.get());
    }

    if (i < maximum_number_of_retries)
    {
      //if it's not the last try, both workers expect to be served a task
      auto const job_received_0 (job_submitted_0.get());
      auto const job_received_1 (job_submitted_1.get());
      BOOST_REQUIRE_EQUAL (job_received_0, job_received_1);
    }

    //restart worker_1
    worker_1 = std::make_unique<utils::fake_drts_worker_notifying_module_call_submission>
      ( [&] (std::string k) { job_submitted_1.put (k); }
      , agent
      , certificates
      );
  }

  worker_0.canceled (cancel_requested_0.get());

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id, job_info), sdpa::status::FAILED);

  BOOST_REQUIRE_EQUAL (job_info.error_message, "Number of retries exceeded!");
}
