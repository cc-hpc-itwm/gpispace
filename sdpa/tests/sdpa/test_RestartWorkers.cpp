#define BOOST_TEST_MODULE restart_drts_worker_while_jobs_are_running

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/tokenizer.hpp>

#include <fhg/util/random_string.hpp>

#include <sdpa/events/CapabilitiesGainedEvent.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace
{
  bool has_state_pending (sdpa::discovery_info_t const& disc_res)
  {
   return disc_res.state() && disc_res.state().get() == sdpa::status::PENDING;
  }

  bool all_childs_are_pending (sdpa::discovery_info_t const& disc_res)
  {
   BOOST_FOREACH
     (const sdpa::discovery_info_t& child_info, disc_res.children())
   {
     if (!has_state_pending (child_info))
     {
       return false;
     }
   }

   return true;
  }

  bool has_children_and_all_children_are_pending
    (sdpa::discovery_info_t const& disc_res)
  {
    std::cout<<disc_res<<std::endl;
    return !disc_res.children().empty() && all_childs_are_pending (disc_res);
  }
}

class Worker : public utils::BasicWorker
{
  public:
    Worker (const std::string& name, const utils::agent& master_agent, const std::string cpb_name = "")
      :  utils::BasicWorker (name, master_agent, cpb_name)
    {}

    void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent* pEvt)
    {
      sdpa::events::SubmitJobAckEvent::Ptr
      pSubmitJobAckEvt(new sdpa::events::SubmitJobAckEvent( _name
                                                          , pEvt->from()
                                                          , *pEvt->job_id()));
      _network_strategy->perform (pSubmitJobAckEvt);

      sdpa::events::JobFinishedEvent::Ptr
      pJobFinishedEvt(new sdpa::events::JobFinishedEvent( _name
                                                        , pEvt->from()
                                                        , *pEvt->job_id()
                                                        , pEvt->description() ));

      _network_strategy->perform (pJobFinishedEvt);
    }

    void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent* ){}
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

  // wait until all remaining jobs are discovered pending
  while (!has_children_and_all_children_are_pending
        (client.discoverJobStates (fhg::util::random_string(), job_id))
          )
  {}

  Worker worker_0( "worker_0", agent);

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

  // wait until all remaining jobs are discovered pending
  while (!has_children_and_all_children_are_pending
         (client.discoverJobStates (fhg::util::random_string(), job_id))
           )
   {}

  Worker worker_0( "worker_0", agent);

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

  const Worker worker_0( "worker_0", agent, "A");

  Worker* pWorker_1(new Worker( "worker_1", agent, "A"));

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  delete pWorker_1;

  // wait until all remaining jobs are discovered pending
  while (!has_children_and_all_children_are_pending
         (client.discoverJobStates (fhg::util::random_string(), job_id)) )
   {}

  const Worker worker_1( "worker_1", agent, "A");

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
        ( client.wait_for_terminal_state (job_id, job_info)
        , sdpa::status::FINISHED );
}
