#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include <sdpa/events/worker_registration_response.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <test/certificates_data.hpp>

#include <util-generic/hostname.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (sdpa::capabilities_set_t)
{
  sdpa::capabilities_set_t capabilities;

  auto count (fhg::util::testing::random<std::size_t>{} (100, 0));
  while (count --> 0)
  {
    capabilities.emplace ( fhg::util::testing::random<std::string>{}()
                         , utils::random_peer_name()
                         );
  }

  return capabilities;
}

namespace
{
  struct drts_component : utils::basic_drts_component_no_logic
  {
    drts_component (fhg::com::Certificates const&);

    template<typename Component>
      fhg::com::p2p::address_t connect_to (Component const& component);
    template<typename Event, typename... Args>
      void perform (fhg::com::p2p::address_t addr, Args&&... args);

    void perform_WorkerRegistrationEvent (fhg::com::p2p::address_t addr);

    template<typename Event>
      using Events
        = fhg::util::threadsafe_queue
            <std::pair<fhg::com::p2p::address_t, Event>>;

    Events<sdpa::events::ErrorEvent> errors;
    Events<sdpa::events::WorkerRegistrationEvent> worker_registrations;
    Events<sdpa::events::worker_registration_response>
      worker_registration_responses;

    virtual void handleErrorEvent
      ( fhg::com::p2p::address_t const& source
      , sdpa::events::ErrorEvent const* event
      ) override;
    virtual void handleWorkerRegistrationEvent
      ( fhg::com::p2p::address_t const& source
      , sdpa::events::WorkerRegistrationEvent const* event
      ) override;
    virtual void handle_worker_registration_response
      ( fhg::com::p2p::address_t const& source
      , sdpa::events::worker_registration_response const* event
      ) override;

    basic_drts_component_no_logic::event_thread _ = {*this};
  };

  drts_component::drts_component (fhg::com::Certificates const& certificates)
    : utils::basic_drts_component_no_logic (certificates)
  {}

  template<typename Component>
    fhg::com::p2p::address_t drts_component::connect_to
      (Component const& component)
  {
    return _network.connect_to (component.host(), component.port());
  }
  template<typename Event, typename... Args>
    void drts_component::perform (fhg::com::p2p::address_t addr, Args&&... args)
  {
    _network.perform<Event> (addr, std::forward<Args> (args)...);
  }

  void drts_component::perform_WorkerRegistrationEvent
    (fhg::com::p2p::address_t addr)
  {
    perform<sdpa::events::WorkerRegistrationEvent>
      ( addr
      , name()
      , fhg::util::testing::random<sdpa::capabilities_set_t>{}()
      , fhg::util::testing::random<unsigned long>{}()
      , fhg::util::testing::random<bool>{}()
      , fhg::util::testing::random<std::string>{}()
      );
  }

  void drts_component::handleErrorEvent
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::ErrorEvent const* event
    )
  {
    errors.put (source, *event);
  }
  void drts_component::handleWorkerRegistrationEvent
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::WorkerRegistrationEvent const* event
    )
  {
    worker_registrations.put (source, *event);
  }
  void drts_component::handle_worker_registration_response
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::worker_registration_response const* event
    )
  {
    worker_registration_responses.put (source, *event);
  }

  auto const avoid_infinite_wait_for_events
    (boost::unit_test::timeout (11));
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_events)
BOOST_DATA_TEST_CASE
  (agent_acknowledges_worker_registration, certificates_data, certificates)
{
  utils::agent const agent (certificates);

  drts_component observer (certificates);

  auto const addr (observer.connect_to (agent));

  observer.perform_WorkerRegistrationEvent (addr);

  auto const response (observer.worker_registration_responses.get());

  BOOST_REQUIRE_EQUAL (response.first, addr);
  BOOST_REQUIRE_NO_THROW (response.second.get());
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_events)
BOOST_DATA_TEST_CASE
  (agent_refuses_second_child_with_same_name, certificates_data, certificates)
{
  utils::agent const agent (certificates);

  drts_component observer (certificates);

  auto const addr (observer.connect_to (agent));

  observer.perform_WorkerRegistrationEvent (addr);
  observer.worker_registration_responses.get();

  observer.perform_WorkerRegistrationEvent (addr);

  auto const response (observer.worker_registration_responses.get());

  BOOST_REQUIRE_EQUAL (response.first, addr);
  fhg::util::testing::require_exception
    ( [&] { response.second.get(); }
    , std::runtime_error ("worker '" + observer.name() + "' already exists")
    );
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_events)
BOOST_DATA_TEST_CASE
  (agent_registes_with_parent_and_disconnects, certificates_data, certificates)
{
  drts_component observer (certificates);

  boost::optional<fhg::com::p2p::address_t> agent_addr;

  {
    utils::agent const agent ({observer.host(), observer.port()}, certificates);

    auto const registration (observer.worker_registrations.get());

    BOOST_REQUIRE_EQUAL (registration.second.name(), agent.name());
    BOOST_REQUIRE_EQUAL
      (registration.second.capabilities(), sdpa::capabilities_set_t{});
    BOOST_REQUIRE_EQUAL (registration.second.hostname(), fhg::util::hostname());
    BOOST_REQUIRE_EQUAL (registration.second.children_allowed(), true);
    BOOST_REQUIRE_EQUAL (registration.second.allocated_shared_memory_size(), 0);

    agent_addr = registration.first;
  }

  auto const disconnect (observer.errors.get());
  BOOST_REQUIRE_EQUAL (disconnect.first, *agent_addr);
  BOOST_REQUIRE_EQUAL ( disconnect.second.error_code()
                      , sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                      );
}
