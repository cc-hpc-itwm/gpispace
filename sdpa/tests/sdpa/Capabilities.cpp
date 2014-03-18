#define BOOST_TEST_MODULE testCapabilities

#include <utils.hpp>

#include <fhg/util/random_string.hpp>

#include <sdpa/events/CapabilitiesLostEvent.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (setup_logging)

namespace
{
  class drts_component_observing_capabilities : public utils::basic_drts_component
  {
  public:
    drts_component_observing_capabilities (utils::kvs_server const& kvs_server)
      : utils::basic_drts_component
        (utils::random_peer_name(), kvs_server, true)
    {}

    virtual void handleCapabilitiesGainedEvent
      (const sdpa::events::CapabilitiesGainedEvent* pEvt)
    {
      boost::mutex::scoped_lock const _ (_mutex);
      BOOST_FOREACH (const sdpa::capability_t& cpb, pEvt->capabilities())
      {
        _capabilities.insert (cpb);
        _capabilitiy_added.notify_all();
      }
    }

    virtual void handleCapabilitiesLostEvent
      (const sdpa::events::CapabilitiesLostEvent* pEvt)
    {
      boost::mutex::scoped_lock const _ (_mutex);
      BOOST_FOREACH (const sdpa::capability_t& cpb, pEvt->capabilities())
      {
        _capabilities.erase (cpb);
        _capabilitiy_added.notify_all();
      }
    }

    void wait_for_capabilities
      (const unsigned int n, const sdpa::capabilities_set_t& expected_cpb_set)
    {
      boost::mutex::scoped_lock lock (_mutex);
      while (_capabilities.size() != n)
      {
        _capabilitiy_added.wait (lock);
      }
      BOOST_REQUIRE (_capabilities == expected_cpb_set);
    }

  private:
    boost::mutex _mutex;
    boost::condition_variable_any _capabilitiy_added;
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
  drts_component_observing_capabilities observer (kvs_server);

  const utils::agent agent (observer);

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

    observer.wait_for_capabilities (2, expected);
  }
}

BOOST_AUTO_TEST_CASE (lose_capabilities_after_worker_dies)
{
  const utils::kvs_server kvs_server;
  drts_component_observing_capabilities observer (kvs_server);

  const utils::agent agent (observer);

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

      observer.wait_for_capabilities (2, expected);
    }
  }

  {
    sdpa::capabilities_set_t expected;
    expected.insert (capability_0);

    observer.wait_for_capabilities (1, expected);
  }
}
