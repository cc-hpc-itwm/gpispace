#define BOOST_TEST_MODULE TestCancelJob

#include "kvs_setup_fixture.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public utils::BasicAgent
{
public:
  Worker (const utils::agent& master_agent)
    : utils::BasicAgent (utils::random_peer_name(), master_agent, "")
  {}

  void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent* pEvt)
  {
    sdpa::events::SubmitJobAckEvent::Ptr pSubmitJobAckEvt
      (new sdpa::events::SubmitJobAckEvent ( _name
                                           , pEvt->from()
                                           , *pEvt->job_id()
                                           )
      );
    _network_strategy->perform (pSubmitJobAckEvt);
    _cond_got_job.notify_one();
  }

  void handleCancelJobEvent (const sdpa::events::CancelJobEvent* pEvt)
  {
    sdpa::events::CancelJobAckEvent::Ptr pEvtCancelAck
      ( new sdpa::events::CancelJobAckEvent ( _name
                                            , pEvt->from()
                                            , pEvt->job_id()
                                            )
      );
    _network_strategy->perform (pEvtCancelAck);
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

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  sdpa::job_id_t job_id (client.submitJob (utils::module_call()));
  client.cancelJob (job_id);

  BOOST_REQUIRE_EQUAL
    ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED
    );
}

BOOST_AUTO_TEST_CASE (cancel_with_agent)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());

  utils::agent agent (orchestrator);

  Worker worker (agent);

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  sdpa::job_id_t job_id (client.submitJob (utils::module_call()));

  worker.wait_for_jobs();

  client.cancelJob (job_id);

  BOOST_REQUIRE_EQUAL
    ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED
    );
}

BOOST_AUTO_TEST_CASE (call_cancel_twice_orch)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  sdpa::job_id_t job_id (client.submitJob (utils::module_call()));
  client.cancelJob (job_id);

  BOOST_REQUIRE_EQUAL
    ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED
    );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (call_cancel_twice_agent)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  utils::agent agent (orchestrator);

  Worker worker (agent);

  sdpa::client::Client client (orchestrator.name(),  kvs_host(), kvs_port());
  sdpa::job_id_t job_id (client.submitJob (utils::module_call()));

  worker.wait_for_jobs();

  client.cancelJob (job_id);

  BOOST_REQUIRE_EQUAL
    ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED
    );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}
