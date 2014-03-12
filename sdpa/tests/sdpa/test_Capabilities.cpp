#define BOOST_TEST_MODULE testCapabilities

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <fhg/util/random_string.hpp>

#include <sdpa/events/CapabilitiesLostEvent.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public utils::BasicWorker
{
  public:
    Worker (const std::string& name
            , const utils::agent& master_agent
            , const std::string cpb_name)
      :  utils::BasicWorker (name, master_agent, cpb_name)
    {}

    void getCapabilities(sdpa::capabilities_set_t& cpbset) { cpbset = _capabilities; }
};

class Master : public utils::BasicWorker
{
  public:
    Master (const std::string& name)
      :  utils::BasicWorker (name, boost::none)
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
  const std::string workflow
    (utils::require_and_read_file ("capabilities.pnet"));

  Master master("orchestrator_0");

  const utils::agent agent
     ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), master);

  Worker worker_0( "worker_0", agent, "A");
  Worker worker_1( "worker_1", agent, "B");

  sdpa::capabilities_set_t set_cpbs_0;
  worker_0.getCapabilities(set_cpbs_0);
  sdpa::capabilities_set_t set_cpbs_1;
  worker_1.getCapabilities(set_cpbs_1);

  sdpa::capabilities_set_t expected_cpbs(set_cpbs_0);
  BOOST_FOREACH(const sdpa::capability_t& cpb, set_cpbs_1)
  {
    expected_cpbs.insert(cpb);
  }

  master.wait_for_capabilities(2, expected_cpbs);
}


BOOST_AUTO_TEST_CASE (lose_capabilities_after_worker_dies)
{
  const std::string workflow
      (utils::require_and_read_file ("capabilities.pnet"));

  Master master("orchestrator_0");

  const utils::agent agent
     ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), master);

  Worker worker_0( "worker_0", agent, "A");
  Worker* pWorker_1(new Worker( "worker_1", agent, "B"));

  sdpa::capabilities_set_t set_cpbs_0;
  worker_0.getCapabilities(set_cpbs_0);
  sdpa::capabilities_set_t set_cpbs_1;
  pWorker_1->getCapabilities(set_cpbs_1);

  sdpa::capabilities_set_t expected_cpbs(set_cpbs_0);
  BOOST_FOREACH(const sdpa::capability_t& cpb, set_cpbs_1)
  {
    expected_cpbs.insert(cpb);
  }

  master.wait_for_capabilities(2, expected_cpbs);

  delete pWorker_1;

  master.wait_for_capabilities(1, set_cpbs_0);
}
