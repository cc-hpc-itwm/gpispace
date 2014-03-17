#define BOOST_TEST_MODULE testCapabilities

#include "kvs_setup_fixture.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <fhg/util/random_string.hpp>

#include <sdpa/events/CapabilitiesLostEvent.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public utils::BasicAgent
{
  public:
    Worker (const std::string& name
            , const utils::agent& master_agent
           , sdpa::capability_t capability)
      :  utils::BasicAgent (name, master_agent, capability)
    {}

    void getCapabilities(sdpa::capabilities_set_t& cpbset) { cpbset = _capabilities; }
};

class Master : public utils::BasicAgent
{
  public:
    Master (const std::string& name)
      :  utils::BasicAgent (name, boost::none, boost::none)
    {}

    void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent* pRegEvt)
    {
      sdpa::events::WorkerRegistrationAckEvent::Ptr
        pRegAckEvt(new sdpa::events::WorkerRegistrationAckEvent (_name, pRegEvt->from()));

      _network_strategy->perform (pRegAckEvt);
    }

   void handleCapabilitiesGainedEvent(const sdpa::events::CapabilitiesGainedEvent* pEvt)
   {
     BOOST_FOREACH(const sdpa::capability_t& cpb, pEvt->capabilities())
    {
       _capabilities.insert(cpb);
       _cond_capabilities.notify_all();
    }
   }

   void handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent* pEvt)
   {
      BOOST_FOREACH(const sdpa::capability_t& cpb, pEvt->capabilities())
     {
        _capabilities.erase (cpb);
        _cond_capabilities.notify_all();
     }
   }

   void handleErrorEvent(const sdpa::events::ErrorEvent*) {}

   const std::string name() const { return _name; }

   void wait_for_capabilities(const unsigned int n, const sdpa::capabilities_set_t& expected_cpb_set)
   {
     boost::unique_lock<boost::mutex> lock_cpbs (_mtx_capabilities);
     while(_capabilities.size() != n)
       _cond_capabilities.wait(lock_cpbs);
     BOOST_REQUIRE(_capabilities == expected_cpb_set);
   }

  private:
   boost::mutex _mtx_capabilities;
   boost::condition_variable_any _cond_capabilities;
};

BOOST_AUTO_TEST_CASE (acquire_capabilities_from_workers)
{
  Master master("orchestrator_0");

  const utils::agent agent
     ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), master);

  const sdpa::capability_t capability_0 ("A", "worker_0");
  const sdpa::capability_t capability_1 ("B", "worker_1");
  Worker worker_0( "worker_0", agent, capability_0);
  Worker worker_1( "worker_1", agent, capability_1);

  sdpa::capabilities_set_t set_cpbs_0;
  set_cpbs_0.insert (capability_0);
  sdpa::capabilities_set_t set_cpbs_1;
  set_cpbs_1.insert (capability_1);

  sdpa::capabilities_set_t expected_cpbs(set_cpbs_0);
  BOOST_FOREACH(const sdpa::capability_t& cpb, set_cpbs_1)
  {
    expected_cpbs.insert(cpb);
  }

  master.wait_for_capabilities(2, expected_cpbs);
}


BOOST_AUTO_TEST_CASE (lose_capabilities_after_worker_dies)
{
  Master master("orchestrator_0");

  const utils::agent agent
     ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), master);

  const sdpa::capability_t capability_0 ("A", "worker_0");
  const sdpa::capability_t capability_1 ("B", "worker_1");
  Worker worker_0( "worker_0", agent, capability_0);
  Worker* pWorker_1(new Worker( "worker_1", agent, capability_1));

  sdpa::capabilities_set_t set_cpbs_0;
  set_cpbs_0.insert (capability_0);
  sdpa::capabilities_set_t set_cpbs_1;
  set_cpbs_1.insert (capability_1);

  sdpa::capabilities_set_t expected_cpbs(set_cpbs_0);
  BOOST_FOREACH(const sdpa::capability_t& cpb, set_cpbs_1)
  {
    expected_cpbs.insert(cpb);
  }

  master.wait_for_capabilities(2, expected_cpbs);

  delete pWorker_1;

  master.wait_for_capabilities(1, set_cpbs_0);
}
