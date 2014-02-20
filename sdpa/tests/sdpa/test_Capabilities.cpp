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
    Worker (const std::string& name, const std::string& master_name, const std::string cpb_name)
      : Agent (name, "127.0.0.1", kvs_host(), kvs_port(), sdpa::master_info_list_t(1, sdpa::MasterInfo(master_name)), boost::none)
    {
        sdpa::capability_t  cpb(cpb_name, name);
        addCapability(cpb);
        sdpa::events::CapabilitiesGainedEvent::Ptr
            ptrCpbGainEvt( new sdpa::events::CapabilitiesGainedEvent(name, master_name, cpb) );
        sendEventToOther(ptrCpbGainEvt);
    }

    void submit ( const we::layer::id_type& activity_id
                , const we::type::activity_t& activity )
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

  Worker worker_0( "worker_0", "agent_0", "A");
  Worker worker_1( "worker_1", "agent_0", "B");

  sdpa::capabilities_set_t set_cpbs_0;
  worker_0.getCapabilities(set_cpbs_0);
  sdpa::capabilities_set_t set_cpbs_1;
  worker_1.getCapabilities(set_cpbs_1);
  sdpa::capabilities_set_t expected_cpbs(set_cpbs_0);
  BOOST_FOREACH(const sdpa::capability_t& cpb, set_cpbs_1)
  {
    expected_cpbs.insert(cpb);
  }

  sdpa::capabilities_set_t acquired_cpbs;
  while(acquired_cpbs.size()!=2)
  {
     acquired_cpbs.clear();
     agent._.getCapabilities(acquired_cpbs);
  }

  BOOST_REQUIRE(acquired_cpbs == expected_cpbs);
}

BOOST_AUTO_TEST_CASE (lose_capabilities)
{
  const std::string workflow
    (utils::require_and_read_file ("capabilities.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  Worker worker_0( "worker_0", "agent_0", "A");
  Worker* pWorker_1(new Worker( "worker_1", "agent_0", "B"));

  sdpa::capabilities_set_t set_cpbs_0;
  worker_0.getCapabilities(set_cpbs_0);

  sdpa::capabilities_set_t acquired_cpbs;
  while (acquired_cpbs.size()!=2)
  {
     acquired_cpbs.clear();
     agent._.getCapabilities(acquired_cpbs);
  }

  delete pWorker_1;

  while (acquired_cpbs.size()!=1)
  {
     acquired_cpbs.clear();
     agent._.getCapabilities(acquired_cpbs);
  }

  BOOST_REQUIRE(acquired_cpbs == set_cpbs_0);
}
