#include <utils.hpp>

#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <functional>

namespace
{
  class fake_drts_worker_notifying_submission_and_cancel final
    : public utils::no_thread::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_notifying_submission_and_cancel
        ( std::function<void (std::string)> announce_job
        , std::function<void (std::string)> announce_cancel
        , const utils::agent& master_agent
        , fhg::com::Certificates const& certificates
        )
      : utils::no_thread::fake_drts_worker_notifying_module_call_submission
        (announce_job, master_agent, certificates)
      , _announce_cancel (announce_cancel)
    {}

    void handleCancelJobEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent* e) override
    {
      const std::map<std::string, job_t>::const_iterator it
        ( std::find_if
          ( _jobs.begin(), _jobs.end()
          , [&e] (std::pair<std::string, job_t> v)
          {
            return v.second._id == e->job_id();
          }
          )
        );
      BOOST_REQUIRE (it != _jobs.end());
      BOOST_REQUIRE_EQUAL (source, it->second._owner);

      _announce_cancel (it->first);
    }

    void canceled (std::string name)
    {
      const job_t job (_jobs.at (name));
      _jobs.erase (name);

      _network.perform<sdpa::events::CancelJobAckEvent> (job._owner, job._id);
    }

  private:
    std::function<void (std::string)> _announce_cancel;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };
}

BOOST_DATA_TEST_CASE_F
  (setup_logging, cancel_no_agent, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (_logger, certificates);

  utils::client client (orchestrator, certificates);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  client.cancel_job (job_id);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);
}

BOOST_DATA_TEST_CASE_F
  (setup_logging, cancel_with_agent, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (_logger, certificates);
  const utils::agent agent (orchestrator, _logger, certificates);

  fhg::util::thread::event<> job_submitted;
  fhg::util::thread::event<std::string> cancel_requested;
  fake_drts_worker_notifying_submission_and_cancel worker
    ( [&job_submitted] (std::string) { job_submitted.notify(); }
    , [&cancel_requested] (std::string j) { cancel_requested.notify (j); }
    , agent
    , certificates
    );

  utils::client client (orchestrator, certificates);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  job_submitted.wait();

  client.cancel_job (job_id);

  worker.canceled (cancel_requested.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);
}

BOOST_DATA_TEST_CASE_F
  (setup_logging, call_cancel_twice_orch, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (_logger, certificates);

  utils::client client (orchestrator, certificates);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  client.cancel_job (job_id);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);

  BOOST_REQUIRE_THROW (client.cancel_job (job_id), std::runtime_error);
}

BOOST_DATA_TEST_CASE_F
  (setup_logging, call_cancel_twice_agent, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (_logger, certificates);
  const utils::agent agent (orchestrator, _logger, certificates);

  fhg::util::thread::event<> job_submitted;
  fhg::util::thread::event<std::string> cancel_requested;
  fake_drts_worker_notifying_submission_and_cancel worker
    ( [&job_submitted] (std::string) { job_submitted.notify(); }
    , [&cancel_requested] (std::string j) { cancel_requested.notify (j); }
    , agent
    , certificates
    );

  utils::client client (orchestrator, certificates);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  job_submitted.wait();

  client.cancel_job (job_id);

  worker.canceled (cancel_requested.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);

  BOOST_REQUIRE_THROW (client.cancel_job (job_id), std::runtime_error);
}

BOOST_DATA_TEST_CASE_F
  (setup_logging, cancel_pending_jobs, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (_logger, certificates);
  const utils::agent agent (orchestrator, _logger, certificates);

  fhg::util::thread::event<> job_submitted;
  utils::fake_drts_worker_notifying_module_call_submission worker
    ( [&job_submitted] (std::string) { job_submitted.notify(); }
    , agent
    , certificates
    );

  utils::client client (orchestrator, certificates);

  const sdpa::job_id_t job_id_0 (client.submit_job (utils::module_call()));
  job_submitted.wait();

  const sdpa::job_id_t job_id_1 (client.submit_job (utils::module_call()));

  client.cancel_job (job_id_1);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id_1), sdpa::status::CANCELED);
}

BOOST_DATA_TEST_CASE_F
  (setup_logging, cancel_workflow_with_two_activities, certificates_data, certificates)
{
  const utils::orchestrator orchestrator (_logger, certificates);
  const utils::agent agent (orchestrator, _logger, certificates);

  fhg::util::thread::event<> job_submitted_0;
  fhg::util::thread::event<std::string> cancel_requested_0;
  fake_drts_worker_notifying_submission_and_cancel worker_0
    ( [&job_submitted_0] (std::string) { job_submitted_0.notify(); }
    , [&cancel_requested_0] (std::string j) { cancel_requested_0.notify (j); }
    , agent
    , certificates
    );

  fhg::util::thread::event<> job_submitted_1;
  fhg::util::thread::event<std::string> cancel_requested_1;
  fake_drts_worker_notifying_submission_and_cancel worker_1
     ( [&job_submitted_1] (std::string) { job_submitted_1.notify(); }
     , [&cancel_requested_1] (std::string j) { cancel_requested_1.notify (j); }
     , agent
     , certificates
     );

  utils::client client (orchestrator, certificates);
  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_two_children_requiring_n_workers (2)));

  job_submitted_0.wait();
  job_submitted_1.wait();

  client.cancel_job (job_id);

  worker_0.canceled (cancel_requested_0.wait());
  worker_1.canceled (cancel_requested_1.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);
}
