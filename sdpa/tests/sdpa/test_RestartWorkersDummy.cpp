#define BOOST_TEST_MODULE restart_worker_with_dummy_workflow

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public utils::BasicAgent
{
  public:
    Worker (const std::string& name, const utils::agent& master_agent, const std::string cpb_name = "")
      :  utils::BasicAgent (name, master_agent, cpb_name)
    {}

    void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent* pEvt)
    {
      sdpa::events::SubmitJobAckEvent::Ptr
      pSubmitJobAckEvt(new sdpa::events::SubmitJobAckEvent( _name
                                                          , pEvt->from()
                                                          , *pEvt->job_id()));
      _network_strategy->perform (pSubmitJobAckEvt);
    }
};

BOOST_AUTO_TEST_CASE (restart_worker_with_dumm_workflow)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);

  sdpa::worker_id_t const worker_id (utils::random_peer_name());
  Worker* pWorker(new Worker(worker_id, agent));

  utils::client::client_t client (orchestrator);
  sdpa::job_id_t const job_id (client.submit_job (utils::module_call()));

  delete pWorker;

  utils::fake_drts_worker_directly_finishing_jobs worker_0 (worker_id, agent);

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
