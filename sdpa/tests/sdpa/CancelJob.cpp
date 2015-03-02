#define BOOST_TEST_MODULE TestCancelJob

#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>

#include <functional>

namespace
{
  class fake_drts_worker_notifying_submission_and_cancel
    : public utils::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_notifying_submission_and_cancel
        ( std::function<void (std::string)> announce_job
        , std::function<void (std::string)> announce_cancel
        , const utils::agent& master_agent
        )
      : utils::fake_drts_worker_notifying_module_call_submission
        (announce_job, master_agent)
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

      _network.perform
        ( job._owner
        , sdpa::events::SDPAEvent::Ptr
            (new sdpa::events::CancelJobAckEvent (job._id))
        );
    }

  private:
    std::function<void (std::string)> _announce_cancel;
  };
}

BOOST_FIXTURE_TEST_CASE (cancel_no_agent, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);

  utils::client client (orchestrator);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  client.cancel_job (job_id);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);
}

BOOST_FIXTURE_TEST_CASE (cancel_with_agent, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator);

  fhg::util::thread::event<> job_submitted;
  fhg::util::thread::event<std::string> cancel_requested;
  fake_drts_worker_notifying_submission_and_cancel worker
    ( [&job_submitted] (std::string) { job_submitted.notify(); }
    , [&cancel_requested] (std::string j) { cancel_requested.notify (j); }
    , agent
    );

  utils::client client (orchestrator);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  job_submitted.wait();

  client.cancel_job (job_id);

  worker.canceled (cancel_requested.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);
}

BOOST_FIXTURE_TEST_CASE (call_cancel_twice_orch, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);

  utils::client client (orchestrator);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  client.cancel_job (job_id);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);

  BOOST_REQUIRE_THROW (client.cancel_job (job_id), std::runtime_error);
}

BOOST_FIXTURE_TEST_CASE (call_cancel_twice_agent, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator);

  fhg::util::thread::event<> job_submitted;
  fhg::util::thread::event<std::string> cancel_requested;
  fake_drts_worker_notifying_submission_and_cancel worker
    ( [&job_submitted] (std::string) { job_submitted.notify(); }
    , [&cancel_requested] (std::string j) { cancel_requested.notify (j); }
    , agent
    );

  utils::client client (orchestrator);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  job_submitted.wait();

  client.cancel_job (job_id);

  worker.canceled (cancel_requested.wait());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);

  BOOST_REQUIRE_THROW (client.cancel_job (job_id), std::runtime_error);
}
