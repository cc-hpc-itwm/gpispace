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
   sdpa::capabilities_set_t _capabilities;
};

BOOST_AUTO_TEST_CASE (acquire_capabilities_from_workers)
{
  Master master("orchestrator_0");

  const utils::agent agent
     ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), master);

  const std::string name_0 (utils::random_peer_name());
  const std::string name_1 (utils::random_peer_name());
  const sdpa::capability_t capability_0 ("A", name_0);
  const sdpa::capability_t capability_1 ("B", name_1);
  Worker worker_0( name_0, agent, capability_0);
  Worker worker_1( name_1, agent, capability_1);

  {
    sdpa::capabilities_set_t expected;
    expected.insert (capability_0);
    expected.insert (capability_1);

    master.wait_for_capabilities (2, expected);
  }
}


BOOST_AUTO_TEST_CASE (lose_capabilities_after_worker_dies)
{
  Master master("orchestrator_0");

  const utils::agent agent
     ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), master);

  const std::string name_0 (utils::random_peer_name());
  const std::string name_1 (utils::random_peer_name());
  const sdpa::capability_t capability_0 ("A", name_0);
  const sdpa::capability_t capability_1 ("B", name_1);
  Worker worker_0( name_0, agent, capability_0);
  Worker* pWorker_1(new Worker( name_1, agent, capability_1));

  {
    sdpa::capabilities_set_t expected;
    expected.insert (capability_0);
    expected.insert (capability_1);

    master.wait_for_capabilities (2, expected);
  }

  delete pWorker_1;

  {
    sdpa::capabilities_set_t expected;
    expected.insert (capability_0);

    master.wait_for_capabilities (1, expected);
  }
}
