#define BOOST_TEST_MODULE restart_drts_worker_while_jobs_are_running

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

#include <fhg/util/random_string.hpp>

class Worker : public utils::BasicWorker
{
  public:
    Worker (const std::string& name, const utils::agent& master_agent, const std::string cpb_name = "", bool notify_finished = false)
      :  utils::BasicWorker (name, master_agent, cpb_name)
      , _notify_finished(notify_finished)
    {}

    void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent* pEvt)
    {
      sdpa::events::SubmitJobAckEvent::Ptr
      pSubmitJobAckEvt(new sdpa::events::SubmitJobAckEvent( _name
                                                          , pEvt->from()
                                                          , *pEvt->job_id()));
      _network_strategy->perform (pSubmitJobAckEvt);

      if(_notify_finished)
      {
          sdpa::events::JobFinishedEvent::Ptr
          pJobFinishedEvt(new sdpa::events::JobFinishedEvent( _name
                                                            , pEvt->from()
                                                            , *pEvt->job_id()
                                                            , pEvt->description() ));

          _network_strategy->perform (pJobFinishedEvt);
      }
    }

    void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent* ){}

  private:
    bool _notify_finished;
};


BOOST_AUTO_TEST_CASE (restart_worker_polling_client)
{
  const std::string workflow
    (utils::require_and_read_file ("dummy_workflow.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  const utils::agent agent
     ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  Worker* pWorker(new Worker("worker_0", agent));

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  delete pWorker;

  Worker worker_0("worker_0", agent, "", true);

  BOOST_REQUIRE_EQUAL
     ( utils::client::wait_for_job_termination(client, job_id)
     , sdpa::status::FINISHED );
}

BOOST_AUTO_TEST_CASE (restart_worker_subscribing_client)
{
  const std::string workflow
    (utils::require_and_read_file ("dummy_workflow.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  const utils::agent agent
       ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  Worker* pWorker(new Worker("worker_0", agent));

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  delete pWorker;

  Worker worker_0("worker_0", agent, "", true);

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
      ( client.wait_for_terminal_state (job_id, job_info)
      , sdpa::status::FINISHED );
}

BOOST_AUTO_TEST_CASE (restart_workers_while_job_requiring_coallocation_is_runnig)
{
  const std::string workflow
    (utils::require_and_read_file ("coallocation.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const Worker worker_0("worker_0", agent, "A", true);

  std::string worker_id_1("worker_1");
  Worker* pWorker_1(new Worker(worker_id_1, agent, "A"));

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  delete pWorker_1;

  const Worker worker_1(worker_id_1, agent, "A", true);

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
        ( client.wait_for_terminal_state (job_id, job_info)
        , sdpa::status::FINISHED );
}
