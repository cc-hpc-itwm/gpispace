#define BOOST_TEST_MODULE testDiscoverJobStates

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <we/layer.hpp>

#include <boost/test/unit_test.hpp>
#include <sdpa/types.hpp>

#include <deque>

#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>

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

  bool has_two_childs_that_are_pending (sdpa::discovery_info_t const& disc_res)
  {
    return disc_res.children().size() == 2 && all_childs_are_pending (disc_res);
  }

  bool has_children_and_all_children_are_pending
    (sdpa::discovery_info_t const& disc_res)
  {
    return !disc_res.children().empty() && all_childs_are_pending (disc_res);
  }
}

class Worker : public utils::BasicWorker
{
  public:
    Worker (const std::string& name, const std::string& master_name, const std::string cpb_name)
      :  utils::BasicWorker (name, master_name, cpb_name)
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

    void remove_capability(const std::string& cpb_name)
    {
      sdpa::capability_t cpb(cpb_name, _name);
      sdpa::events::CapabilitiesLostEvent::Ptr
        ptrCpbLostEvt( new sdpa::events::CapabilitiesLostEvent(_name, _master_name, cpb) );
      _network_strategy->perform (ptrCpbLostEvt);
   }
};

BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace
{
  std::string get_next_disc_id()
  {
    static int i (0);

    return (boost::format ("discover_%1%") % i++).str();
  }
}

BOOST_AUTO_TEST_CASE (discover_discover_inexistent_job)
{
  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  BOOST_REQUIRE_EQUAL
    ( client.discoverJobStates ("disc_id_0", "inexistent_job_id").state()
    , boost::none
    );
}

BOOST_AUTO_TEST_CASE (discover_one_orchestrator_no_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("dummy_workflow.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  while (!has_state_pending
          (client.discoverJobStates (get_next_disc_id(), job_id))
        )
  {}; // do nothing, discover again
}

BOOST_AUTO_TEST_CASE (discover_one_orchestrator_one_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("dummy_workflow.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);
  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  while (!all_childs_are_pending
          (client.discoverJobStates (get_next_disc_id(), job_id))
        )
  {} // do nothing, discover again
}

BOOST_AUTO_TEST_CASE (insufficient_number_of_workers)
{
  // this workflow produces 2 activities
  // one requires 2 workers, the other 3 workers
  const std::string workflow
    (utils::require_and_read_file ("coallocation_test2.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const Worker worker_0( "worker_0", agent._.name(), "A");
  const Worker worker_1( "worker_1", agent._.name(), "B");
  const Worker worker_2( "worker_2", agent._.name(), "B");

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  while (!has_two_childs_that_are_pending
          (client.discoverJobStates (get_next_disc_id(), job_id))
        )
  {} // do nothing, discover again
}

BOOST_AUTO_TEST_CASE (discover_after_losing_capabilities)
{
  const std::string workflow
    (utils::require_and_read_file ("coallocation_test2.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  const utils::agent agent
     ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  Worker worker_0( "worker_0", agent._.name(), "A");
  Worker worker_1( "worker_1", agent._.name(), "B");
  const Worker worker_2( "worker_2", agent._.name(), "A");
  const Worker worker_3( "worker_3", agent._.name(), "B");
  const Worker worker_4( "worker_4", agent._.name(), "B");

  // the task A requires 2 workers, task B requires 3 workers
  // initially, there are sufficient resources and the workflow computation should finish
  BOOST_REQUIRE_EQUAL
      ( utils::client::submit_job_and_wait_for_termination_as_subscriber
        (workflow, orchestrator)
      , sdpa::status::FINISHED
      );

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  worker_0.remove_capability("A");
  worker_1.remove_capability("B");

  while (!has_children_and_all_children_are_pending
          (client.discoverJobStates (get_next_disc_id(), job_id))
        )
  {} // do nothing, discover again
}

BOOST_AUTO_TEST_CASE (discover_after_removing_workers)
{
  const std::string workflow
    (utils::require_and_read_file ("coallocation_test2.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  const utils::agent agent
     ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  Worker*  pWorker_0 = new Worker( "worker_0", agent._.name(), "A");
  Worker*  pWorker_1 = new Worker( "worker_1", agent._.name(), "B");
  const Worker worker_2( "worker_2", agent._.name(), "A");
  const Worker worker_3( "worker_3", agent._.name(), "B");
  const Worker worker_4( "worker_4", agent._.name(), "B");


  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (workflow));

  delete pWorker_0;
  delete pWorker_1;

  while (!has_children_and_all_children_are_pending
          (client.discoverJobStates (get_next_disc_id(), job_id))
        )
  {} // do nothing, discover again
}
