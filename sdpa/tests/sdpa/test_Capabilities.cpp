#define BOOST_TEST_MODULE testCapabilities

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <fhg/util/random_string.hpp>

#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace {
  bool operator==(const we::type::requirement_t& left, const we::type::requirement_t& right)
  {
    return ( left.value() == right.value() ) && ( left.is_mandatory() == right.is_mandatory() );
  }

  const we::type::requirement_t req_A("A", true);
  const we::type::requirement_t req_B("B", true);

  // this workflow has 20 tasks of type A and 10 tasks of type B (see CMakeLists)
  const int n_total_expectd_activities  = 30;
}

namespace sdpa
{
  namespace daemon
  {
    class TestAgent : public Agent
    {
    public:
      TestAgent (const std::string& name)
        : Agent (name, "127.0.0.1", kvs_host(), kvs_port(), sdpa::master_info_list_t(), boost::none)
          , _n_recv_tasks_A(0), _n_recv_tasks_B(0)
      {}

      void submit ( const we::layer::id_type&
                  , const we::type::activity_t& activity
                  )
      {
        const std::list<we::type::requirement_t> list_req( activity.transition().requirements() );

        BOOST_REQUIRE(list_req.front()==req_A || list_req.front()==req_B);

        boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
        if(list_req.front()==req_A) {_n_recv_tasks_A++;}
        if(list_req.front()==req_B) {_n_recv_tasks_B++;}

        if (n_recv_tasks_A() + n_recv_tasks_B() == n_total_expectd_activities )
        {
          _cond_all_submitted.notify_one();
        }
      }

      void wait_all_submitted()
      {
        boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
        _cond_all_submitted.wait (_mtx_all_submitted);
      }

      unsigned int n_recv_tasks_A() const { return _n_recv_tasks_A;}
      unsigned int n_recv_tasks_B() const { return _n_recv_tasks_B;}

    private:

      boost::mutex _mtx_all_submitted;
      boost::condition_variable_any _cond_all_submitted;
      unsigned int _n_recv_tasks_A;
      unsigned int _n_recv_tasks_B;
    };
  }
}

BOOST_AUTO_TEST_CASE (check_requested_capabilities_are_valid)
{
  // this workflow has 20 tasks of type A and 10 tasks of type B
  const std::string workflow
    (utils::require_and_read_file ("workflows/capabilities.pnet"));

  const we::type::activity_t activity (workflow);
  sdpa::daemon::TestAgent agent ("agent_0");

  agent.workflowEngine()->submit (fhg::util::random_string(), activity);
  agent.wait_all_submitted();

  BOOST_REQUIRE_EQUAL(agent.n_recv_tasks_A(), 20);
  BOOST_REQUIRE_EQUAL(agent.n_recv_tasks_B(), 10);
}

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
