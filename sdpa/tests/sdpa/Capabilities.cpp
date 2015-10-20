#define BOOST_TEST_MODULE testCapabilities

#include <utils.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random_string.hpp>

#include <sdpa/events/CapabilitiesLostEvent.hpp>

#include <boost/test/unit_test.hpp>

#include <condition_variable>
#include <map>
#include <mutex>

namespace
{
  class drts_component_observing_capabilities : public utils::basic_drts_component
  {
  public:
    drts_component_observing_capabilities()
      : utils::basic_drts_component (utils::random_peer_name(), true)
    {}

    virtual void handleCapabilitiesGainedEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::CapabilitiesGainedEvent* event) override
    {
      std::unique_lock<std::mutex> const _ (_mutex);
      for (const sdpa::capability_t& cpb : event->capabilities())
      {
        _capabilities.emplace_back (source, cpb);
      }
      _capabilities_changed.notify_one();
    }

    virtual void handleCapabilitiesLostEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::CapabilitiesLostEvent* event) override
    {
      std::unique_lock<std::mutex> const _ (_mutex);
      for (const sdpa::capability_t& cpb : event->capabilities())
      {
        _capabilities.erase
          ( std::find ( _capabilities.begin()
                      , _capabilities.end()
                      , decltype (_capabilities)::value_type (source, cpb)
                      )
          );
      }
      _capabilities_changed.notify_one();
    }

    virtual void handleErrorEvent
      ( fhg::com::p2p::address_t const& source
      , sdpa::events::ErrorEvent const* event
      ) override
    {
      if (event->error_code() == sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN)
      {
        //! \note hack? isn't this part of what this test is supposed
        //! to test?
        std::unique_lock<std::mutex> const _ (_mutex);
        _capabilities.remove_if
          ( [&] (decltype (_capabilities)::value_type const& elem)
            {
              return elem.first == source;
            }
          );
        _capabilities_changed.notify_one();
      }

      utils::basic_drts_component::handleErrorEvent (source, event);
    }

    void wait_for_capabilities
      (sdpa::capabilities_set_t const& expected)
    {
      std::unique_lock<std::mutex> lock (_mutex);
      _capabilities_changed.wait
        (lock, [&] { return _capabilities.size() == expected.size(); });

      sdpa::capabilities_set_t capabilities;
      for ( auto const& capability
          : _capabilities | boost::adaptors::map_values
          )
      {
        capabilities.emplace (capability);
      }
      BOOST_REQUIRE (capabilities == expected);
    }

  private:
    std::mutex _mutex;
    std::condition_variable _capabilities_changed;
    std::list<std::pair<fhg::com::p2p::address_t, sdpa::capability_t>>
      _capabilities;
  };
}

BOOST_FIXTURE_TEST_CASE (acquire_capabilities_from_workers, setup_logging)
{
  drts_component_observing_capabilities observer;

  {
    const utils::agent agent (observer, _logger);

    const std::string name_0 (utils::random_peer_name());
    const std::string name_1 (utils::random_peer_name());
    const sdpa::capability_t capability_0 ("A", name_0);
    const sdpa::capability_t capability_1 ("B", name_1);
    utils::basic_drts_worker const worker_0 (name_0, agent, {capability_0});
    utils::basic_drts_worker const worker_1 (name_1, agent, {capability_1});

    observer.wait_for_capabilities ({capability_0, capability_1});
  }

  observer.wait_for_capabilities ({});
}

BOOST_FIXTURE_TEST_CASE (lose_capabilities_after_worker_dies, setup_logging)
{
  drts_component_observing_capabilities observer;

  {
    const utils::agent agent (observer, _logger);

    const std::string name_0 (utils::random_peer_name());
    const std::string name_1 (utils::random_peer_name());
    const sdpa::capability_t capability_0 ("A", name_0);
    const sdpa::capability_t capability_1 ("B", name_1);
    utils::basic_drts_worker const worker_0 (name_0, agent, {capability_0});
    {
      utils::basic_drts_worker const worker_1 (name_1, agent, {capability_1});

      observer.wait_for_capabilities ({capability_0, capability_1});
    }

    observer.wait_for_capabilities ({capability_0});
  }

  observer.wait_for_capabilities ({});
}
