// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
