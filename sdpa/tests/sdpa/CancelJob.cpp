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
  Worker (const utils::agent& master_agent)
    : utils::fake_drts_worker_notifying_module_call_submission
      ( boost::bind (&boost::condition_variable_any::notify_one, &_cond_got_job)
      , master_agent
      )
  {}

  void handleCancelJobEvent (const sdpa::events::CancelJobEvent* pEvt)
  {
    sdpa::events::CancelJobAckEvent::Ptr pEvtCancelAck
      ( new sdpa::events::CancelJobAckEvent ( _name
                                            , pEvt->from()
                                            , pEvt->job_id()
                                            )
      );
    _network.perform (pEvtCancelAck);
  }

  void wait_for_jobs()
  {
    boost::unique_lock<boost::mutex> lock (_mtx_got_job);
    _cond_got_job.wait (lock);
  }

private:
  boost::mutex _mtx_got_job;
  boost::condition_variable_any _cond_got_job;
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

  Worker worker (agent);

  utils::client::client_t client (orchestrator);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  worker.wait_for_jobs();

  client.cancel_job (job_id);

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

  Worker worker (agent);

  utils::client::client_t client (orchestrator);

  const sdpa::job_id_t job_id (client.submit_job (utils::module_call()));

  worker.wait_for_jobs();

  client.cancel_job (job_id);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::CANCELED);

  BOOST_REQUIRE_THROW (client.cancel_job (job_id), std::runtime_error);
}
