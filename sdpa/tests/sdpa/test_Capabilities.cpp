#define BOOST_TEST_MODULE testCapabilities

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <fhg/util/random_string.hpp>

#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public sdpa::daemon::Agent
{
  public:
    Worker (const std::string& name, const std::string& master_name)
      : Agent (name, "127.0.0.1", kvs_host(), kvs_port(), sdpa::master_info_list_t(1, sdpa::MasterInfo(master_name)), boost::none)
    {}

    Worker (const std::string& name,const utils::agents_t& masters)
          : Agent (name, "127.0.0.1", kvs_host(), kvs_port(), utils::assemble_master_info_list (masters), boost::none)
        {}

    void submit ( const we::layer::id_type& activity_id
                , const we::type::activity_t& activity
                )
    {
      sdpa::daemon::GenericDaemon::submit(activity_id, activity);
      workflowEngine()->finished(activity_id, activity);
    }
};

BOOST_AUTO_TEST_CASE (acquire_capabilities_from_workers)
{
  const std::string workflow
    (utils::require_and_read_file ("capabilities.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  Worker worker_0( "worker_0", "agent_0");
  Worker worker_1( "worker_1", "agent_0");

  while(!agent._.hasWorker(worker_1.name()));
  while(!agent._.hasWorker(worker_0.name()));

  sdpa::capability_t cpbA("A", "worker_0");
  sdpa::events::CapabilitiesGainedEvent::Ptr
    ptrCpbGainEvtA( new sdpa::events::CapabilitiesGainedEvent(worker_0.name(), "agent_0", cpbA) );
  agent._.handleCapabilitiesGainedEvent(ptrCpbGainEvtA.get());

  sdpa::capability_t cpbB("B", "worker_1");
  sdpa::events::CapabilitiesGainedEvent::Ptr
    ptrCpbGainEvtB(new sdpa::events::CapabilitiesGainedEvent(worker_1.name(), "agent_0", cpbB));
  agent._.handleCapabilitiesGainedEvent(ptrCpbGainEvtB.get());

  sdpa::capabilities_set_t expected_cpb_set;
  expected_cpb_set.insert(cpbA);
  expected_cpb_set.insert(cpbB);

  sdpa::capabilities_set_t cpbset_worker_0;
  agent._.getWorkerCapabilities(worker_0.name(), cpbset_worker_0);

  sdpa::capabilities_set_t cpbset_worker_1;
  agent._.getWorkerCapabilities(worker_1.name(), cpbset_worker_1);

  sdpa::capabilities_set_t acquired_cpbs;
  agent._.getCapabilities(acquired_cpbs);

  BOOST_REQUIRE(acquired_cpbs==expected_cpb_set);
}

BOOST_AUTO_TEST_CASE (lose_capabilities)
{
  const std::string workflow
     (utils::require_and_read_file ("workflows/capabilities.pnet"));

  const utils::orchestrator orchestrator
     ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  utils::agent agent
     ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  Worker worker_0( "worker_0", "agent_0");
  Worker worker_1( "worker_1", "agent_0");

  while(!agent._.hasWorker(worker_1.name()));
  while(!agent._.hasWorker(worker_0.name()));

  sdpa::capability_t cpbA("A", "worker_0");
  sdpa::events::CapabilitiesGainedEvent::Ptr
    ptrCpbGainEvtA( new sdpa::events::CapabilitiesGainedEvent(worker_0.name(), "agent_0", cpbA) );
  agent._.handleCapabilitiesGainedEvent(ptrCpbGainEvtA.get());

  sdpa::capability_t cpbB("B", "worker_1");
  sdpa::events::CapabilitiesGainedEvent::Ptr
    ptrCpbGainEvtB(new sdpa::events::CapabilitiesGainedEvent(worker_1.name(), "agent_0", cpbB));
  agent._.handleCapabilitiesGainedEvent(ptrCpbGainEvtB.get());

  sdpa::capabilities_set_t expected_cpb_set;
  expected_cpb_set.insert(cpbB);

  sdpa::events::CapabilitiesLostEvent::Ptr
    ptrCpbLostEvtB(new sdpa::events::CapabilitiesLostEvent(worker_0.name(), "agent_0", cpbA));
  agent._.handleCapabilitiesLostEvent(ptrCpbLostEvtB.get());

  sdpa::capabilities_set_t acquired_cpbs;
  agent._.getCapabilities(acquired_cpbs);
  BOOST_REQUIRE(acquired_cpbs==expected_cpb_set);
}
