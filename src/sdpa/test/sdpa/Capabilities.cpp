#include <utils.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>

#include <sdpa/events/CapabilitiesLostEvent.hpp>

#include <boost/test/unit_test.hpp>

#include <condition_variable>
#include <map>
#include <mutex>

namespace
{
  class drts_component_observing_capabilities final : public utils::basic_drts_component
  {
  public:
    drts_component_observing_capabilities (fhg::com::certificates_t const& certificates)
      : utils::basic_drts_component (utils::random_peer_name(), true, certificates)
    {}

    virtual void handleCapabilitiesGainedEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::CapabilitiesGainedEvent* event) override
    {
      BOOST_REQUIRE (!event->capabilities().empty());

      std::lock_guard<std::mutex> const _ (_mutex);
      for (const sdpa::capability_t& cpb : event->capabilities())
      {
        _capabilities.emplace_back (source, cpb);
      }
      _capabilities_changed.notify_one();
    }

    virtual void handleCapabilitiesLostEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::CapabilitiesLostEvent* event) override
    {
      std::lock_guard<std::mutex> const _ (_mutex);
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
      if ( event->error_code() == sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN
         || event->error_code() == sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
         )
      {
        //! \note hack? isn't this part of what this test is supposed
        //! to test?
        std::lock_guard<std::mutex> const _ (_mutex);
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
        BOOST_REQUIRE (capabilities.emplace (capability).second);
      }
      BOOST_REQUIRE (capabilities == expected);
    }

    void wait_for_capabilities (std::size_t count)
    {
      std::unique_lock<std::mutex> lock (_mutex);
      _capabilities_changed.wait
        (lock, [&] { return _capabilities.size() == count; });
    }

  private:
    std::mutex _mutex;
    std::condition_variable _capabilities_changed;
    std::list<std::pair<fhg::com::p2p::address_t, sdpa::capability_t>>
      _capabilities;

    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };
}

void test_acquire_capability_from_worker
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  drts_component_observing_capabilities observer (certificates);

  {
    const utils::agent agent (observer, _logger, certificates);

    const std::string name (utils::random_peer_name());
    const sdpa::capability_t capability ("A", name);
    utils::basic_drts_worker const worker (name, agent, {capability}, certificates);

    observer.wait_for_capabilities ({capability});
  }

  observer.wait_for_capabilities ({});
}

BOOST_TEST_DECORATOR (*boost::unit_test::timeout (2))
BOOST_FIXTURE_TEST_CASE (acquire_capability_from_worker, setup_logging)
{
  test_acquire_capability_from_worker (_logger, boost::none);

  if (test_certificates)
  {
    test_acquire_capability_from_worker (_logger, test_certificates);
  }
}

void test_acquire_capability_from_worker_chain
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  drts_component_observing_capabilities observer (certificates);

  {
    const utils::agent agent_0 (observer, _logger, certificates);
    const utils::agent agent_1 (agent_0, _logger, certificates);

    const std::string name (utils::random_peer_name());
    const sdpa::capability_t capability ("A", name);
    utils::basic_drts_worker const worker (name, agent_1, {capability}, certificates);

    observer.wait_for_capabilities ({capability});
 }

 observer.wait_for_capabilities ({});
}
BOOST_TEST_DECORATOR (*boost::unit_test::timeout (2))
BOOST_FIXTURE_TEST_CASE (acquire_capability_from_worker_chain, setup_logging)
{
  test_acquire_capability_from_worker_chain (_logger, boost::none);

  if (test_certificates)
  {
    test_acquire_capability_from_worker_chain (_logger, test_certificates);
  }
}

void test_acquire_capabilities_from_workers
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  drts_component_observing_capabilities observer (certificates);

  {
    const utils::agent agent (observer, _logger, certificates);

    const std::string name_0 (utils::random_peer_name());
    const std::string name_1 (utils::random_peer_name());
    const sdpa::capability_t capability_0 ("A", name_0);
    const sdpa::capability_t capability_1 ("B", name_1);
    utils::basic_drts_worker const worker_0 (name_0, agent, {capability_0}, certificates);
    utils::basic_drts_worker const worker_1 (name_1, agent, {capability_1}, certificates);

    observer.wait_for_capabilities ({capability_0, capability_1});
  }

  observer.wait_for_capabilities ({});
}
BOOST_FIXTURE_TEST_CASE (acquire_capabilities_from_workers, setup_logging)
{
  test_acquire_capabilities_from_workers (_logger, boost::none);

  if (test_certificates)
  {
    test_acquire_capabilities_from_workers (_logger, test_certificates);
  }
}

void test_lose_capabilities_after_worker_dies
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  drts_component_observing_capabilities observer (certificates);

  {
    const utils::agent agent (observer, _logger, certificates);

    const std::string name_0 (utils::random_peer_name());
    const std::string name_1 (utils::random_peer_name());
    const sdpa::capability_t capability_0 ("A", name_0);
    const sdpa::capability_t capability_1 ("B", name_1);
    utils::basic_drts_worker const worker_0 (name_0, agent, {capability_0}, certificates);
    {
      utils::basic_drts_worker const worker_1 (name_1, agent, {capability_1}, certificates);

      observer.wait_for_capabilities ({capability_0, capability_1});
    }

    observer.wait_for_capabilities ({capability_0});
  }

  observer.wait_for_capabilities ({});
}

BOOST_TEST_DECORATOR (*boost::unit_test::timeout (2))
BOOST_FIXTURE_TEST_CASE (lose_capabilities_after_worker_dies, setup_logging)
{
  test_lose_capabilities_after_worker_dies (_logger, boost::none);

  if (test_certificates)
  {
    test_lose_capabilities_after_worker_dies (_logger, test_certificates);
  }
}

void test_capabilities_of_children_are_removed_when_disconnected
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  //! \note race exists due to us not being able to ensure that no
  //! CapabilitiesLost is sent in the agent chain and only network
  //! errors happen. In fact, the race is rather in the other
  //! direction, though: when a chain of agents exits,
  //! capabilitieslosts are very rare in what I was able to observe
  //! (throw on explicit capabilities_lost). With multiple agents all
  //! exiting as fast as possible, we trigger a disconnect-only quite
  //! likely. Note that the race would not fail the test! :(

  size_t repeat (10);
  while (repeat --> 0)
  {
    drts_component_observing_capabilities observer (certificates);
    utils::agent const agent_0 (observer, _logger, certificates);
    utils::agent const agent_1 (agent_0, _logger, certificates);
    utils::agent const agent_2 (agent_1, _logger, certificates);
    utils::agent const agent_3 (agent_2, _logger, certificates);

    {
      utils::agent const agent_4 (agent_3, _logger, certificates);
      utils::agent const agent_5 (agent_4, _logger, certificates);
      utils::agent const agent_6 (agent_5, _logger, certificates);
      utils::agent const agent_7 (agent_6, _logger, certificates);

      std::string const worker_name (utils::random_peer_name());
      sdpa::capability_t const capability ("A", worker_name);
      utils::basic_drts_worker const worker
        (worker_name, agent_7, {capability}, certificates);

      observer.wait_for_capabilities ({capability});
    }

    observer.wait_for_capabilities ({});
  }
}

BOOST_FIXTURE_TEST_CASE
  ( RACE_capabilities_of_children_are_removed_when_disconnected
  , setup_logging
  )
{
  test_capabilities_of_children_are_removed_when_disconnected (_logger, boost::none);

  if (test_certificates)
  {
    test_capabilities_of_children_are_removed_when_disconnected (_logger, test_certificates);
  }
}

void test_chain_with_a_lot_of_leafs_different_capabilities
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  drts_component_observing_capabilities observer (certificates);

  {
    utils::agent const agent_0 (observer, _logger, certificates);
    utils::agent const agent_1 (agent_0, _logger, certificates);

    std::list<utils::basic_drts_worker> workers;

    std::size_t const count (128);

    for (std::size_t i (0); i < count; ++i)
    {
      sdpa::worker_id_t const name (utils::random_peer_name());
      sdpa::capability_t const capability (std::to_string (count), name);
      workers.emplace_back
        (name, agent_1, sdpa::capabilities_set_t {capability}, certificates);
    }

    observer.wait_for_capabilities (count);
  }

  observer.wait_for_capabilities ({});
}

BOOST_FIXTURE_TEST_CASE ( chain_with_a_lot_of_leafs_different_capabilities
                        , setup_logging
                        )
{
  test_chain_with_a_lot_of_leafs_different_capabilities (_logger, boost::none);

  if (test_certificates)
  {
    test_chain_with_a_lot_of_leafs_different_capabilities (_logger, test_certificates);
  }
}
