// Copyright (C) 2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/scheduler/events/ErrorEvent.hpp>
#include <gspc/scheduler/events/WorkerRegistrationEvent.hpp>
#include <gspc/scheduler/events/worker_registration_response.hpp>
#include <test/scheduler/scheduler/utils.hpp>
#include <gspc/scheduler/types.hpp>

#include <gspc/testing/certificates_data.hpp>

#include <gspc/util/hostname.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/generic.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/printer/set.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/util/threadsafe_queue.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>

GSPC_TESTING_RANDOM_SPECIALIZE_SIMPLE (gspc::scheduler::Capabilities)
{
  gspc::scheduler::Capabilities capabilities;

  auto count (gspc::testing::random<std::size_t>{} (100, 0));
  while (count --> 0)
  {
    capabilities.emplace (gspc::testing::random<std::string>{}());
  }

  return capabilities;
}

namespace
{
  struct drts_component : utils::basic_drts_component_no_logic
  {
    drts_component (gspc::Certificates const&);

    template<typename Component>
      gspc::com::p2p::address_t connect_to_TESTING_ONLY (Component const& component);
    template<typename Event, typename... Args>
      void perform (gspc::com::p2p::address_t addr, Args&&... args);

    void perform_WorkerRegistrationEvent (gspc::com::p2p::address_t addr);

    gspc::util::threadsafe_queue
      <std::pair< gspc::com::p2p::address_t
                , std::optional<std::exception_ptr>
                >
      > worker_registration_responses;

    void handle_worker_registration_response
      ( gspc::com::p2p::address_t const& source
      , gspc::scheduler::events::worker_registration_response const* event
      ) override;

    basic_drts_component_no_logic::event_thread _ = {*this};
  };

  drts_component::drts_component (gspc::Certificates const& certificates)
    : utils::basic_drts_component_no_logic (certificates)
  {}

  template<typename Component>
    gspc::com::p2p::address_t drts_component::connect_to_TESTING_ONLY
      (Component const& component)
  {
    return _network.connect_to_TESTING_ONLY (component.host(), component.port());
  }
  template<typename Event, typename... Args>
    void drts_component::perform (gspc::com::p2p::address_t addr, Args&&... args)
  {
    _network.perform<Event> (addr, std::forward<Args> (args)...);
  }

  void drts_component::perform_WorkerRegistrationEvent
    (gspc::com::p2p::address_t addr)
  {
    perform<gspc::scheduler::events::WorkerRegistrationEvent>
      ( addr
      , name()
      , gspc::testing::random<gspc::scheduler::Capabilities>{}()
      , gspc::testing::random<unsigned long>{}()
      , gspc::testing::random<std::string>{}()
      );
  }

  void drts_component::handle_worker_registration_response
    ( gspc::com::p2p::address_t const& source
    , gspc::scheduler::events::worker_registration_response const* event
    )
  {
    worker_registration_responses.put (source, event->exception());
  }

  auto const avoid_infinite_wait_for_events
    (::boost::unit_test::timeout (11));
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_events)
BOOST_DATA_TEST_CASE
  (agent_acknowledges_worker_registration, certificates_data, certificates)
{
  utils::agent const agent (certificates);

  drts_component observer (certificates);

  auto const addr (observer.connect_to_TESTING_ONLY (agent));

  observer.perform_WorkerRegistrationEvent (addr);

  auto const response (observer.worker_registration_responses.get());

  BOOST_REQUIRE (response.first == addr);
  BOOST_REQUIRE (!response.second);
}

BOOST_TEST_DECORATOR (*avoid_infinite_wait_for_events)
BOOST_DATA_TEST_CASE
  (agent_refuses_second_child_with_same_name, certificates_data, certificates)
{
  utils::agent const agent (certificates);

  drts_component observer (certificates);

  auto const addr (observer.connect_to_TESTING_ONLY (agent));

  observer.perform_WorkerRegistrationEvent (addr);
  observer.worker_registration_responses.get();

  observer.perform_WorkerRegistrationEvent (addr);

  auto const response (observer.worker_registration_responses.get());

  BOOST_REQUIRE (response.first == addr);
  BOOST_REQUIRE (!!response.second);
  gspc::testing::require_exception
    ( [&] { std::rethrow_exception (*response.second); }
    , std::runtime_error ("worker '" + observer.name() + "' already exists")
    );
}
