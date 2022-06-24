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

#include <boost/test/unit_test.hpp>

#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/test/NetworkStrategy.hpp>

#include <testing/certificates_data.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>

namespace
{
  struct wait_for_n_events_strategy
  {
    wait_for_n_events_strategy (unsigned int expected)
      : _expected (expected)
    {}

    void perform ( fhg::com::p2p::address_t const&
                 , ::boost::shared_ptr<sdpa::events::SDPAEvent> const&
                 )
    {
      std::lock_guard<std::mutex> _ (_counter_mutex);
      ++_counter;

      if (_counter == _expected)
      {
        _expected_count_reached.notify_all();
      }
      else if (_counter > _expected)
      {
        throw std::logic_error ("got more events than expected");
      }
    }
    void wait() const
    {
      std::unique_lock<std::mutex> _ (_counter_mutex);

      _expected_count_reached.wait (_, [&] { return _counter >= _expected; });

      BOOST_REQUIRE_EQUAL (_counter, _expected);
    }

    mutable std::mutex _counter_mutex;
    mutable std::condition_variable _expected_count_reached;
    unsigned int _counter {0};
    unsigned int _expected;
  };
}

BOOST_DATA_TEST_CASE (test_strategy, certificates_data, certificates)
{
  wait_for_n_events_strategy counter (1);

  sdpa::test::NetworkStrategy net
    ( std::bind ( &wait_for_n_events_strategy::perform
                , &counter
                , std::placeholders::_1
                , std::placeholders::_2
                )
    , std::make_unique<::boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t (0)
    , certificates
    );

  net.perform<sdpa::events::ErrorEvent>
    ( net.connect_to_TESTING_ONLY
        ( fhg::com::host_t ( fhg::util::connectable_to_address_string
                               (net.local_endpoint().address())
                           )
        , fhg::com::port_t (net.local_endpoint().port())
        )
    , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
    , "success"
    );

  counter.wait();
}
