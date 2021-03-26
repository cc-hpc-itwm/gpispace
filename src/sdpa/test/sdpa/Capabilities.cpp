// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <test/certificates_data.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <iterator>
#include <string>
#include <utility>

namespace
{
  class drts_component_observing_capabilities final
    : public utils::basic_drts_component
  {
  public:
    drts_component_observing_capabilities (fhg::com::Certificates const&);

    template<typename Event>
      using Events
        = fhg::util::threadsafe_queue
            <std::pair<fhg::com::p2p::address_t, Event>>;

    Events<sdpa::events::CapabilitiesGainedEvent> capabilities_gained;
    Events<sdpa::events::CapabilitiesLostEvent> capabilities_lost;
    Events<sdpa::events::WorkerRegistrationEvent> worker_registrations;

    virtual void handleCapabilitiesGainedEvent
      ( fhg::com::p2p::address_t const&
      , sdpa::events::CapabilitiesGainedEvent const*
      ) override;
    virtual void handleCapabilitiesLostEvent
      ( fhg::com::p2p::address_t const&
      , sdpa::events::CapabilitiesLostEvent const*
      ) override;
    virtual void handleWorkerRegistrationEvent
      ( fhg::com::p2p::address_t const&
      , sdpa::events::WorkerRegistrationEvent const*
      ) override;

  private:
    utils::basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  class basic_drts_worker_with_public_perform
    : public utils::no_thread::basic_drts_worker
  {
  public:
    basic_drts_worker_with_public_perform
      ( utils::agent const& master
      , sdpa::capabilities_set_t
      , fhg::com::Certificates const&
      );

    template<typename Event, typename... Args>
      void perform_to_master (Args&&... args);

  private:
    utils::basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  drts_component_observing_capabilities::drts_component_observing_capabilities
      (fhg::com::Certificates const& certificates)
    : utils::basic_drts_component (true, certificates)
  {}

  void drts_component_observing_capabilities::handleCapabilitiesGainedEvent
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::CapabilitiesGainedEvent const* event
    )
  {
    capabilities_gained.put (source, *event);
  }
  void drts_component_observing_capabilities::handleCapabilitiesLostEvent
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::CapabilitiesLostEvent const* event
    )
  {
    capabilities_lost.put (source, *event);
  }
  void drts_component_observing_capabilities::handleWorkerRegistrationEvent
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::WorkerRegistrationEvent const* event
    )
  {
    worker_registrations.put (source, *event);
    return basic_drts_component::handleWorkerRegistrationEvent (source, event);
  }

  basic_drts_worker_with_public_perform::basic_drts_worker_with_public_perform
      ( utils::agent const& master
      , sdpa::capabilities_set_t capabilities
      , fhg::com::Certificates const& certificates
      )
    : utils::no_thread::basic_drts_worker (master, capabilities, certificates)
  {}

  template<typename Event, typename... Args>
    void basic_drts_worker_with_public_perform::perform_to_master
      (Args&&... args)
  {
    _network.perform<Event> (_master.get(), args...);
  }

  sdpa::capabilities_set_t random_capabilities()
  {
    sdpa::capabilities_set_t capabilities;

    auto count (fhg::util::testing::random<std::size_t>{} (100, 1));
    while (count --> 0)
    {
      capabilities.emplace ( fhg::util::testing::random<std::string>{}()
                           , utils::random_peer_name()
                           );
    }

    return capabilities;
  }

  auto const avoid_infinite_wait_for_capabilities_call
    (boost::unit_test::timeout (11));
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_capabilities_call)
BOOST_DATA_TEST_CASE
  (agent_has_no_capabilities_at_start, certificates_data, certificates)
{
  drts_component_observing_capabilities observer (certificates);

  utils::agent const agent (observer, certificates);

  BOOST_REQUIRE_EQUAL
    ( observer.worker_registrations.get().second.capabilities()
    , sdpa::capabilities_set_t{}
    );
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_capabilities_call)
BOOST_DATA_TEST_CASE
  (agent_forwards_capabilities_gained, certificates_data, certificates)
{
  drts_component_observing_capabilities observer (certificates);

  utils::agent const agent (observer, certificates);
  observer.worker_registrations.get();

  auto const capabilities (random_capabilities());

  // Sends given capabilities in registration up to agent in ctor.
  utils::basic_drts_worker child (agent, capabilities, certificates);

  BOOST_REQUIRE_EQUAL
    ( observer.capabilities_gained.get().second.capabilities()
    , capabilities
    );
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_capabilities_call)
BOOST_DATA_TEST_CASE
  (agent_notifies_capabilities_lost, certificates_data, certificates)
{
  drts_component_observing_capabilities observer (certificates);

  utils::agent const agent (observer, certificates);
  observer.worker_registrations.get();

  auto const capabilities (random_capabilities());

  basic_drts_worker_with_public_perform child
    (agent, capabilities, certificates);
  observer.capabilities_gained.get();

  child.perform_to_master<sdpa::events::CapabilitiesLostEvent> (capabilities);

  BOOST_REQUIRE_EQUAL
    ( observer.capabilities_lost.get().second.capabilities()
    , capabilities
    );
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_capabilities_call)
BOOST_DATA_TEST_CASE
  (agent_notifies_capabilities_lost_on_crash, certificates_data, certificates)
{
  drts_component_observing_capabilities observer (certificates);

  utils::agent const agent (observer, certificates);
  observer.worker_registrations.get();

  auto const capabilities (random_capabilities());

  {
    utils::basic_drts_worker child (agent, capabilities, certificates);
    observer.capabilities_gained.get();

    // \note Explicitly no capabilities lost: network error instead.
  }

  BOOST_REQUIRE_EQUAL
    ( observer.capabilities_lost.get().second.capabilities()
    , capabilities
    );
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_capabilities_call)
BOOST_DATA_TEST_CASE
  (agent_notifies_capabilities_lost_two_childs, certificates_data, certificates)
{
  drts_component_observing_capabilities observer (certificates);

  utils::agent const agent (observer, certificates);
  observer.worker_registrations.get();

  auto const capabilities_a (random_capabilities());
  auto const capabilities_b (random_capabilities());

  utils::basic_drts_worker child_a (agent, capabilities_a, certificates);

  BOOST_REQUIRE_EQUAL
    ( observer.capabilities_gained.get().second.capabilities()
    , capabilities_a
    );

  {
    utils::basic_drts_worker child_b (agent, capabilities_b, certificates);

    BOOST_REQUIRE_EQUAL
      ( observer.capabilities_gained.get().second.capabilities()
      , capabilities_b
      );
  }

  BOOST_REQUIRE_EQUAL
    ( observer.capabilities_lost.get().second.capabilities()
    , capabilities_b
    );
}

namespace
{
  template<typename Set>
    Set random_non_empty_subset (Set& set)
  {
    Set subset;

    fhg::util::testing::random<std::size_t> random_size_t;

    auto count (random_size_t (set.size(), 1));
    while (count --> 0)
    {
      auto const to_remove
        (std::next (set.begin(), random_size_t (set.size() - 1)));
      subset.emplace (*to_remove);
      set.erase (to_remove);
    }

    return subset;
  }
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_capabilities_call)
BOOST_DATA_TEST_CASE
  (agent_notifies_capabilities_lost_subset, certificates_data, certificates)
{
  drts_component_observing_capabilities observer (certificates);

  utils::agent const agent (observer, certificates);
  observer.worker_registrations.get();

  auto capabilities (random_capabilities());

  basic_drts_worker_with_public_perform child
    (agent, capabilities, certificates);
  observer.capabilities_gained.get();

  while (!capabilities.empty())
  {
    auto const subset (random_non_empty_subset (capabilities));

    child.perform_to_master<sdpa::events::CapabilitiesLostEvent> (subset);

    BOOST_REQUIRE_EQUAL
      (observer.capabilities_lost.get().second.capabilities(), subset);
  }
}

// \todo Test case: agent does not forward capabilities already
// known. How can that happen though, as the agent has no capabilities
// itself. Cycles? Diamonds?

// \todo Test case: something with agent chains?
