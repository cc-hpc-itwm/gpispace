#define BOOST_TEST_MODULE restart_worker_with_coallocation_workflow

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public utils::BasicAgent
{
  public:
    Worker (const std::string& name, const utils::agent& master_agent, const std::string cpb_name = "", bool notify_finished = false)
      :  utils::BasicAgent (name, master_agent, cpb_name)
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

BOOST_AUTO_TEST_CASE (restart_workers_while_job_requiring_coallocation_is_running)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (kvs_host(), kvs_port(), orchestrator);


  const Worker worker_0 (utils::random_peer_name(), agent, "A", true);

  sdpa::worker_id_t worker_id_1 (utils::random_peer_name());
  Worker* pWorker_1(new Worker(worker_id_1, agent, "A"));

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id
    (client.submitJob (utils::net_with_one_child_requiring_workers (2).to_string()));

  delete pWorker_1;

  const Worker worker_1(worker_id_1, agent, "A", true);

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
        ( client.wait_for_terminal_state (job_id, job_info)
        , sdpa::status::FINISHED );
}
