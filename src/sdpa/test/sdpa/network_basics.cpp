// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/WorkerRegistrationEvent.hpp>
#include <sdpa/events/worker_registration_response.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <testing/certificates_data.hpp>

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

FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE (sdpa::Capabilities)
{
  sdpa::Capabilities capabilities;

  auto count (fhg::util::testing::random<std::size_t>{} (100, 0));
  while (count --> 0)
  {
    capabilities.emplace (fhg::util::testing::random<std::string>{}());
  }

  return capabilities;
}

namespace
{
  struct drts_component : utils::basic_drts_component_no_logic
  {
    drts_component (fhg::com::Certificates const&);

    template<typename Component>
      fhg::com::p2p::address_t connect_to_TESTING_ONLY (Component const& component);
    template<typename Event, typename... Args>
      void perform (fhg::com::p2p::address_t addr, Args&&... args);

    void perform_WorkerRegistrationEvent (fhg::com::p2p::address_t addr);

    fhg::util::threadsafe_queue
      <std::pair< fhg::com::p2p::address_t
                , ::boost::optional<std::exception_ptr>
                >
      > worker_registration_responses;

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
    fhg::com::p2p::address_t drts_component::connect_to_TESTING_ONLY
      (Component const& component)
  {
    return _network.connect_to_TESTING_ONLY (component.host(), component.port());
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
      , fhg::util::testing::random<sdpa::Capabilities>{}()
      , fhg::util::testing::random<unsigned long>{}()
      , fhg::util::testing::random<std::string>{}()
      );
  }

  void drts_component::handle_worker_registration_response
    ( fhg::com::p2p::address_t const& source
    , sdpa::events::worker_registration_response const* event
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
  fhg::util::testing::require_exception
    ( [&] { std::rethrow_exception (*response.second); }
    , std::runtime_error ("worker '" + observer.name() + "' already exists")
    );
}
