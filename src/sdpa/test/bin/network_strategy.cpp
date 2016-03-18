#include <boost/test/unit_test.hpp>

#include <iostream>

#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/cxx14/make_unique.hpp>

#include <boost/thread.hpp>

#include <boost/asio/io_service.hpp>

#include <functional>

namespace
{
  struct wait_for_n_events_strategy
  {
    wait_for_n_events_strategy (unsigned int expected)
      : _counter (0)
      , _expected (expected)
    {}

    void perform ( fhg::com::p2p::address_t const&
                 , const boost::shared_ptr<sdpa::events::SDPAEvent>&
                 )
    {
      boost::mutex::scoped_lock _ (_counter_mutex);
      ++_counter;

      BOOST_REQUIRE_LE (_counter, _expected);
      if (_counter == _expected)
      {
        _expected_count_reached.notify_all();
      }
    }
    void wait() const
    {
      boost::mutex::scoped_lock _ (_counter_mutex);

      _expected_count_reached.wait (_, [&] { return _counter >= _expected; });

      BOOST_REQUIRE_EQUAL (_counter, _expected);
    }

    mutable boost::mutex _counter_mutex;
    mutable boost::condition_variable _expected_count_reached;
    unsigned int _counter;
    unsigned int _expected;
  };
}

BOOST_AUTO_TEST_CASE (perform_test)
{
  wait_for_n_events_strategy counter (1);

  sdpa::com::NetworkStrategy net
    ( std::bind ( &wait_for_n_events_strategy::perform
                , &counter
                , std::placeholders::_1
                , std::placeholders::_2
                )
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  net.perform<sdpa::events::ErrorEvent>
    ( net.connect_to
        ( fhg::com::host_t ( fhg::util::connectable_to_address_string
                               (net.local_endpoint().address())
                           )
        , fhg::com::port_t (std::to_string (net.local_endpoint().port()))
        )
    , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
    , "success"
    );

  counter.wait();
}
