#define BOOST_TEST_MODULE TestCancelJob

#include "kvs_setup_fixture.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public utils::fake_drts_worker_notifying_module_call_submission
{
public:
  Worker ( boost::function<void (std::string)> announce_cancel
         , const utils::agent& master_agent
         )
    : utils::fake_drts_worker_notifying_module_call_submission
      ( boost::bind (&boost::condition_variable_any::notify_one, &_cond_got_job)
      , master_agent
      )
    , _announce_cancel (announce_cancel)
  {}

  void handleCancelJobEvent (const sdpa::events::CancelJobEvent* e)
  {
    const std::map<std::string, job_t>::const_iterator it
      ( std::find_if ( _jobs.begin(), _jobs.end()
                     , boost::bind (&Worker::job_id_matches, _1, e->job_id())
                     )
      );
    BOOST_REQUIRE (it != _jobs.end());
    BOOST_REQUIRE_EQUAL (e->from(), it->second._owner);

    _announce_cancel (it->first);
  }

  void canceled (std::string name)
  {
    const job_t job (_jobs.at (name));
    _jobs.erase (name);

    _network.perform
      ( sdpa::events::SDPAEvent::Ptr
        (new sdpa::events::CancelJobAckEvent (_name, job._owner, job._id))
      );
  }

  void wait_for_jobs()
  {
    boost::unique_lock<boost::mutex> lock (_mtx_got_job);
    _cond_got_job.wait (lock);
  }

private:
  static bool job_id_matches (std::pair<std::string, job_t> v, sdpa::job_id_t id)
  {
    return v.second._id == id;
  }

  boost::mutex _mtx_got_job;
  boost::condition_variable_any _cond_got_job;

  boost::function<void (std::string)> _announce_cancel;
};


BOOST_AUTO_TEST_CASE (cancel_no_agent)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());

  utils::client::client_t client (orchestrator);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  //! \todo Race.

  client.cancel_job (job_id);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);
}

BOOST_AUTO_TEST_CASE (cancel_with_agent)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);

  fhg::util::thread::event<std::string> cancel_requested;
  Worker worker
    ( boost::bind (&fhg::util::thread::event<std::string>::notify, &cancel_requested, _1)
    , agent
    );

  utils::client::client_t client (orchestrator);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  worker.wait_for_jobs();

  client.cancel_job (job_id);

  std::string cancel_name;
  cancel_requested.wait (cancel_name);
  worker.canceled (cancel_name);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);
}

BOOST_AUTO_TEST_CASE (call_cancel_twice_orch)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);

  utils::client::client_t client (orchestrator);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  //! \todo Race.

  client.cancel_job (job_id);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);

  BOOST_REQUIRE_THROW (client.cancel_job (job_id), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (call_cancel_twice_agent)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);

  fhg::util::thread::event<std::string> cancel_requested;
  Worker worker
    ( boost::bind (&fhg::util::thread::event<std::string>::notify, &cancel_requested, _1)
    , agent
    );

  utils::client::client_t client (orchestrator);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  worker.wait_for_jobs();

  client.cancel_job (job_id);

  std::string cancel_name;
  cancel_requested.wait (cancel_name);
  worker.canceled (cancel_name);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);

  BOOST_REQUIRE_THROW (client.cancel_job (job_id), std::runtime_error);
}
