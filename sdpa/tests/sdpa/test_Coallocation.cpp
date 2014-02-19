#define BOOST_TEST_MODULE testCoallocation

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <sdpa/events/CapabilitiesGainedEvent.hpp>

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

BOOST_AUTO_TEST_CASE (testCoallocationWorkflow)
{
  const std::string workflow
    (utils::require_and_read_file ("coallocation_test.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const Worker worker_0( "worker_0", agent._.name(), "A");
  const Worker worker_1( "worker_1", agent._.name(), "A");
  const Worker worker_2( "worker_2", agent._.name(), "B");
  const Worker worker_3( "worker_3", agent._.name(), "B");
  const Worker worker_4( "worker_4", agent._.name(), "B");

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_wait_for_termination_as_subscriber
      (workflow, orchestrator)
    , sdpa::status::FINISHED
    );
}
