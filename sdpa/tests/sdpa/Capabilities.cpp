#define BOOST_TEST_MODULE testCapabilities

#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <fhg/util/random_string.hpp>

#include <sdpa/events/CapabilitiesLostEvent.hpp>

BOOST_GLOBAL_FIXTURE (setup_logging)

namespace
{
  class BasicAgent : public sdpa::events::EventHandler
                   , boost::noncopyable
  {
  public:
    BasicAgent ( std::string kvs_host, std::string kvs_port
               , std::string name
               , boost::optional<const utils::agent&> master_agent
               , boost::optional<sdpa::capability_t> capability
               )
      : _name (name)
      , _kvs_host (kvs_host)
      , _kvs_port (kvs_port)
      , _kvs_client
        ( new fhg::com::kvs::client::kvsc
          (_kvs_host, _kvs_port, true, boost::posix_time::seconds(120), 1)
        )
      , _network_strategy
        ( new sdpa::com::NetworkStrategy ( boost::bind (&BasicAgent::sendEventToSelf, this, _1)
                                         , name
                                         , fhg::com::host_t ("127.0.0.1")
                                         , fhg::com::port_t ("0")
                                         , _kvs_client
                                         )
        )
      , _event_handling_allowed(true)
    {
      sdpa::capabilities_set_t _capabilities;
      if (capability)
      {
        _capabilities.insert (*capability);
      }

      if(master_agent)
      {
        sdpa::events::WorkerRegistrationEvent::Ptr
          pEvtWorkerReg (new sdpa::events::WorkerRegistrationEvent( _name
                                                                  , master_agent->name()
                                                                  , boost::none
                                                                  , _capabilities ));
        _network_strategy->perform (pEvtWorkerReg);
      }
    }

    virtual ~BasicAgent() { _event_handling_allowed = false; }

    virtual void sendEventToSelf(const sdpa::events::SDPAEvent::Ptr& pEvt)
    {
      if(_event_handling_allowed)
        pEvt->handleBy (this);
    }

    void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent*){}


    std::string kvs_host() const { return _kvs_host; }
    std::string kvs_port() const { return _kvs_port; }

  protected:
    std::string _name;
    std::string _kvs_host;
    std::string _kvs_port;

    fhg::com::kvs::kvsc_ptr_t _kvs_client;
    boost::shared_ptr<sdpa::com::NetworkStrategy> _network_strategy;
    bool _event_handling_allowed;
  };

  template<typename T> std::set<T> set (T v)
  {
    std::set<T> s;
    s.insert (v);
    return s;
  }

  class Worker : public utils::basic_drts_worker
  {
  public:
    Worker ( const utils::agent& master_agent
           , std::string name
           , sdpa::capability_t capability
           )
      : utils::basic_drts_worker (name, master_agent, set (capability))
    {}
  };
}

class Master : public BasicAgent
{
  public:
    Master (std::string kvs_host, std::string kvs_port)
      :  BasicAgent (kvs_host, kvs_port, utils::random_peer_name(), boost::none, boost::none)
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
  const utils::kvs_server kvs_server;
  Master master (kvs_server.kvs_host(), kvs_server.kvs_port());

  const utils::agent agent (master);

  const std::string name_0 (utils::random_peer_name());
  const std::string name_1 (utils::random_peer_name());
  const sdpa::capability_t capability_0 ("A", name_0);
  const sdpa::capability_t capability_1 ("B", name_1);
  Worker worker_0( agent, name_0, capability_0);
  Worker worker_1( agent, name_1, capability_1);

  {
    sdpa::capabilities_set_t expected;
    expected.insert (capability_0);
    expected.insert (capability_1);

    master.wait_for_capabilities (2, expected);
  }
}


BOOST_AUTO_TEST_CASE (lose_capabilities_after_worker_dies)
{
  const utils::kvs_server kvs_server;
  Master master (kvs_server.kvs_host(), kvs_server.kvs_port());

  const utils::agent agent (master);

  const std::string name_0 (utils::random_peer_name());
  const std::string name_1 (utils::random_peer_name());
  const sdpa::capability_t capability_0 ("A", name_0);
  const sdpa::capability_t capability_1 ("B", name_1);
  Worker worker_0( agent, name_0, capability_0);
  {
    Worker pWorker_1( agent, name_1, capability_1);

    {
      sdpa::capabilities_set_t expected;
      expected.insert (capability_0);
      expected.insert (capability_1);

      master.wait_for_capabilities (2, expected);
    }
  }

  {
    sdpa::capabilities_set_t expected;
    expected.insert (capability_0);

    master.wait_for_capabilities (1, expected);
  }
}
