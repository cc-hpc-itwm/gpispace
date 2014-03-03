#define BOOST_TEST_MODULE testCapabilities

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <fhg/util/random_string.hpp>

#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class Worker : public sdpa::events::EventHandler,
                    boost::noncopyable
{
public:
  Worker( std::string name
               , std::string master_name
               , std::string cpb_name
               )
   : _name (name)
   , _master_name(master_name)
   , _kvs_client
     ( new fhg::com::kvs::client::kvsc
       (kvs_host(), kvs_port(), true, boost::posix_time::seconds(120), 1)
     )
   , _network_strategy
     ( new sdpa::com::NetworkStrategy ( boost::bind (&Worker::sendEventToSelf, this, _1)
                                      , name
                                      , fhg::com::host_t ("127.0.0.1")
                                      , fhg::com::port_t ("0")
                                      , _kvs_client
                                      )
     )
  {
    sdpa::capability_t cpb(cpb_name, name);
    _capabilities.insert(cpb);

    sdpa::events::WorkerRegistrationEvent::Ptr
      pEvtWorkerReg (new sdpa::events::WorkerRegistrationEvent( _name
                                                                , _master_name
                                                                , boost::none
                                                                , _capabilities ));
    _network_strategy->perform (pEvtWorkerReg);

    sdpa::events::CapabilitiesGainedEvent::Ptr
      ptrCpbGainEvt( new sdpa::events::CapabilitiesGainedEvent(name, master_name, cpb) );
    _network_strategy->perform (ptrCpbGainEvt);
  }

  void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr&) {}
  void getCapabilities(sdpa::capabilities_set_t& cpbset) { cpbset = _capabilities; }

private:
  std::string _name;
  std::string _master_name;
  sdpa::capabilities_set_t _capabilities;
  fhg::com::kvs::kvsc_ptr_t _kvs_client;
  boost::shared_ptr<sdpa::com::NetworkStrategy> _network_strategy;
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

BOOST_AUTO_TEST_CASE (lose_capabilities_after_worker_dies)
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
