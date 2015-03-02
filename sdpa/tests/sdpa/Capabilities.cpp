#define BOOST_TEST_MODULE testCapabilities

#include <utils.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <fhg/util/random_string.hpp>

#include <sdpa/events/CapabilitiesLostEvent.hpp>

#include <boost/test/unit_test.hpp>

namespace
{
  class drts_component_observing_capabilities : public utils::basic_drts_component
  {
  public:
    drts_component_observing_capabilities (fhg::log::Logger& logger)
      : utils::basic_drts_component (utils::random_peer_name(), true, logger)
    {}

    virtual void handleCapabilitiesGainedEvent
      (fhg::com::p2p::address_t const&, const sdpa::events::CapabilitiesGainedEvent* event) override
    {
      boost::mutex::scoped_lock const _ (_mutex);
      for (const sdpa::capability_t& cpb : event->capabilities())
      {
        _capabilities.insert (cpb);
        _capabilitiy_added.notify_all();
      }
    }

    virtual void handleCapabilitiesLostEvent
      (fhg::com::p2p::address_t const&, const sdpa::events::CapabilitiesLostEvent* event) override
    {
      boost::mutex::scoped_lock const _ (_mutex);
      for (const sdpa::capability_t& cpb : event->capabilities())
      {
        _capabilities.erase (cpb);
        _capabilitiy_added.notify_all();
      }
    }

    void wait_for_capabilities
      (const unsigned int n, const sdpa::capabilities_set_t& expected)
    {
      boost::mutex::scoped_lock lock (_mutex);
      while (_capabilities.size() != n)
      {
        _capabilitiy_added.wait (lock);
      }
      BOOST_REQUIRE (_capabilities == expected);
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

BOOST_FIXTURE_TEST_CASE (acquire_capabilities_from_workers, setup_logging)
{
  drts_component_observing_capabilities observer (_logger);

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

BOOST_FIXTURE_TEST_CASE (lose_capabilities_after_worker_dies, setup_logging)
{
  drts_component_observing_capabilities observer (_logger);

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
