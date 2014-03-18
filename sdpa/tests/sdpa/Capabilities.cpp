#define BOOST_TEST_MODULE testCapabilities

#include <utils.hpp>

#include <fhg/util/random_string.hpp>

#include <sdpa/events/CapabilitiesLostEvent.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (setup_logging)

namespace
{
  class Master : public utils::basic_drts_component
  {
  public:
    Master (utils::kvs_server const& kvs_server)
      : utils::basic_drts_component
        (utils::random_peer_name(), kvs_server, true)
    {}

    virtual void handleCapabilitiesGainedEvent
      (const sdpa::events::CapabilitiesGainedEvent* pEvt)
    {
      BOOST_FOREACH (const sdpa::capability_t& cpb, pEvt->capabilities())
      {
        _capabilities.insert (cpb);
        _cond_capabilities.notify_all();
      }
    }

    virtual void handleCapabilitiesLostEvent
      (const sdpa::events::CapabilitiesLostEvent* pEvt)
    {
      BOOST_FOREACH (const sdpa::capability_t& cpb, pEvt->capabilities())
      {
        _capabilities.erase (cpb);
        _cond_capabilities.notify_all();
      }
    }

    void wait_for_capabilities
      (const unsigned int n, const sdpa::capabilities_set_t& expected_cpb_set)
    {
      boost::unique_lock<boost::mutex> lock_cpbs (_mtx_capabilities);
      while (_capabilities.size() != n)
      {
        _cond_capabilities.wait (lock_cpbs);
      }
      BOOST_REQUIRE (_capabilities == expected_cpb_set);
    }

  private:
    boost::mutex _mtx_capabilities;
    boost::condition_variable_any _cond_capabilities;
    sdpa::capabilities_set_t _capabilities;
  };

  template<typename T> std::set<T> set (T v)
  {
    std::set<T> s;
    s.insert (v);
    return s;
  }
}

BOOST_AUTO_TEST_CASE (acquire_capabilities_from_workers)
{
  const utils::kvs_server kvs_server;
  Master master (kvs_server);

  const utils::agent agent (master);

  const std::string name_0 (utils::random_peer_name());
  const std::string name_1 (utils::random_peer_name());
  const sdpa::capability_t capability_0 ("A", name_0);
  const sdpa::capability_t capability_1 ("B", name_1);
  utils::basic_drts_worker worker_0 (name_0, agent, set (capability_0));
  utils::basic_drts_worker worker_1 (name_1, agent, set (capability_1));

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
  Master master (kvs_server);

  const utils::agent agent (master);

  const std::string name_0 (utils::random_peer_name());
  const std::string name_1 (utils::random_peer_name());
  const sdpa::capability_t capability_0 ("A", name_0);
  const sdpa::capability_t capability_1 ("B", name_1);
  utils::basic_drts_worker worker_0 (name_0, agent, set (capability_0));
  {
    utils::basic_drts_worker pWorker_1 (name_1, agent, set (capability_1));

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
