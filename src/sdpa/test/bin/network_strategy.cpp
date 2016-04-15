#include <boost/test/unit_test.hpp>

#include <iostream>

#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/random_string.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/thread.hpp>

#include <boost/asio/io_service.hpp>

#include <functional>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (fhg::com::p2p::address_t, os, address)
{
  os << fhg::com::p2p::to_string (address);
}

BOOST_AUTO_TEST_CASE (address_translation_equal)
{
  BOOST_CHECK_EQUAL ( fhg::com::p2p::address_t ("host", 1)
                    , fhg::com::p2p::address_t ("host", 1)
                    );
}

BOOST_AUTO_TEST_CASE (address_translation_unique)
{
  BOOST_CHECK_NE ( fhg::com::p2p::address_t ("name1", 1)
                 , fhg::com::p2p::address_t ("name2", 1)
                 );
  BOOST_CHECK_NE ( fhg::com::p2p::address_t ("name", 1)
                 , fhg::com::p2p::address_t ("name", 2)
                 );
}

namespace
{
  void disallow_events
    ( fhg::com::p2p::address_t const&
    , boost::shared_ptr<sdpa::events::SDPAEvent> const&
    )
  {
    BOOST_FAIL ("no events allowed in this peer");
  }

  void ignore_network_failure
    (fhg::com::p2p::address_t const&, std::exception_ptr const&)
  {
  }

  fhg::com::host_t host (boost::asio::ip::tcp::endpoint const& ep)
  {
    return fhg::com::host_t
      (fhg::util::connectable_to_address_string (ep.address()));
  }
  fhg::com::port_t port (boost::asio::ip::tcp::endpoint const& ep)
  {
    return fhg::com::port_t (std::to_string (ep.port()));
  }
}

BOOST_TEST_DECORATOR (*boost::unit_test::timeout (2))
BOOST_AUTO_TEST_CASE (ctor_does_not_hang_when_resolve_throws)
{
  fhg::util::testing::require_exception
    ( []
      {
        sdpa::com::NetworkStrategy
          ( &disallow_events
          , &ignore_network_failure
          , fhg::util::cxx14::make_unique<boost::asio::io_service>()
          , fhg::com::host_t ("NONONONONONONONONONO")
          , fhg::com::port_t ("NONONONONONONONONONO")
          );
      }
    , boost::system::system_error
        (boost::asio::error::service_not_found, "resolve")
    );
}

BOOST_AUTO_TEST_CASE (connect_to_nonexisting_peer)
{
  sdpa::com::NetworkStrategy net
    ( &disallow_events
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  BOOST_CHECK_THROW ( net.connect_to ( fhg::com::host_t ("unknown host")
                                     , fhg::com::port_t ("unknown service")
                                     )
                    , std::exception
                    );
}

BOOST_AUTO_TEST_CASE (identifiable_addresses)
{
  sdpa::com::NetworkStrategy peer_1
    ( &disallow_events
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  sdpa::com::NetworkStrategy peer_2
    ( &disallow_events
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  sdpa::com::NetworkStrategy peer_3
    ( &disallow_events
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  BOOST_REQUIRE_EQUAL ( peer_2.connect_to ( host (peer_1.local_endpoint())
                                          , port (peer_1.local_endpoint())
                                          )
                      , peer_3.connect_to ( host (peer_1.local_endpoint())
                                          , port (peer_1.local_endpoint())
                                          )
                      );
}

BOOST_TEST_DECORATOR (*boost::unit_test::timeout (2))
BOOST_AUTO_TEST_CASE (network_failure_handler_is_called_on_disconnect)
{
  fhg::util::thread::event<> network_failure_called;

  sdpa::com::NetworkStrategy peer_1
    ( &disallow_events
    , [&] (fhg::com::p2p::address_t const&, std::exception_ptr const&)
      {
        network_failure_called.notify();
      }
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  {
    sdpa::com::NetworkStrategy peer_2
      ( &disallow_events
      , [&] (fhg::com::p2p::address_t const&, std::exception_ptr const&)
        {
          BOOST_FAIL ("peer 2 should never get an error: peer 1 outlives");
        }
      , fhg::util::cxx14::make_unique<boost::asio::io_service>()
      , fhg::com::host_t ("localhost")
      , fhg::com::port_t ("0")
      );

    //! \note Should also work the other way around, but connecting is
    //! slower than destructing peer 2, thus would require a sleep and
    //! be a race.
    //! \todo Test the inverse connect_to as well.
    peer_1.connect_to ( host (peer_2.local_endpoint())
                      , port (peer_2.local_endpoint())
                      );
  }

  network_failure_called.wait();
}

BOOST_AUTO_TEST_CASE (ping)
{
  std::string const content (fhg::util::testing::random_string());

  sdpa::com::NetworkStrategy peer_1
    ( &disallow_events
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  fhg::util::thread::event<boost::shared_ptr<sdpa::events::SDPAEvent>> event;
  sdpa::com::NetworkStrategy peer_2
    ( [&] ( fhg::com::p2p::address_t const&
          , boost::shared_ptr<sdpa::events::SDPAEvent> const& received
          )
      {
        event.notify (received);
      }
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  peer_1.perform<sdpa::events::ErrorEvent>
    ( peer_1.connect_to ( host (peer_2.local_endpoint())
                        , port (peer_2.local_endpoint())
                        )
    , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
    , content
    );

  boost::shared_ptr<sdpa::events::SDPAEvent> raw_event (event.wait());
  sdpa::events::ErrorEvent* error_event
    (dynamic_cast<sdpa::events::ErrorEvent*> (raw_event.get()));
  BOOST_REQUIRE (error_event);
  BOOST_REQUIRE_EQUAL
    (error_event->error_code(), sdpa::events::ErrorEvent::SDPA_EUNKNOWN);
  BOOST_REQUIRE_EQUAL (error_event->reason(), content);
}

BOOST_AUTO_TEST_CASE (ping_pong)
{
  std::string const content (fhg::util::testing::random_string());

  fhg::util::thread::event<boost::shared_ptr<sdpa::events::SDPAEvent>> event;

  sdpa::com::NetworkStrategy peer_1
    ( [&] ( fhg::com::p2p::address_t const&
          , boost::shared_ptr<sdpa::events::SDPAEvent> const& received
          )
      {
        event.notify (received);
      }
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  sdpa::com::NetworkStrategy peer_2
    ( [&] ( fhg::com::p2p::address_t const& source
          , boost::shared_ptr<sdpa::events::SDPAEvent> const& received
          )
      {
        sdpa::events::ErrorEvent* error_event
          (dynamic_cast<sdpa::events::ErrorEvent*> (received.get()));
        BOOST_REQUIRE (error_event);

        peer_2.perform<sdpa::events::ErrorEvent>
          (source, error_event->error_code(), error_event->reason());
      }
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  peer_1.perform<sdpa::events::ErrorEvent>
    ( peer_1.connect_to ( host (peer_2.local_endpoint())
                        , port (peer_2.local_endpoint())
                        )
    , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
    , content
    );

  boost::shared_ptr<sdpa::events::SDPAEvent> raw_event (event.wait());
  sdpa::events::ErrorEvent* error_event
    (dynamic_cast<sdpa::events::ErrorEvent*> (raw_event.get()));
  BOOST_REQUIRE (error_event);
  BOOST_REQUIRE_EQUAL
    (error_event->error_code(), sdpa::events::ErrorEvent::SDPA_EUNKNOWN);
  BOOST_REQUIRE_EQUAL (error_event->reason(), content);
}

BOOST_AUTO_TEST_CASE (large_event)
{
  std::string const content (2 << 25, 'X');

  sdpa::com::NetworkStrategy peer_1
    ( &disallow_events
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  fhg::util::thread::event<boost::shared_ptr<sdpa::events::SDPAEvent>> event;
  sdpa::com::NetworkStrategy peer_2
    ( [&] ( fhg::com::p2p::address_t const&
          , boost::shared_ptr<sdpa::events::SDPAEvent> const& received
          )
      {
        event.notify (received);
      }
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  peer_1.perform<sdpa::events::ErrorEvent>
    ( peer_1.connect_to ( host (peer_2.local_endpoint())
                        , port (peer_2.local_endpoint())
                        )
    , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
    , content
    );

  boost::shared_ptr<sdpa::events::SDPAEvent> raw_event (event.wait());
  sdpa::events::ErrorEvent* error_event
    (dynamic_cast<sdpa::events::ErrorEvent*> (raw_event.get()));
  BOOST_REQUIRE (error_event);
  BOOST_REQUIRE_EQUAL
    (error_event->error_code(), sdpa::events::ErrorEvent::SDPA_EUNKNOWN);
  BOOST_REQUIRE_EQUAL (error_event->reason(), content);
}

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

BOOST_AUTO_TEST_CASE (many_to_self)
{
  std::size_t n (10231);
  wait_for_n_events_strategy counter (n);

  sdpa::com::NetworkStrategy net
    ( std::bind ( &wait_for_n_events_strategy::perform
                , &counter
                , std::placeholders::_1
                , std::placeholders::_2
                )
    , &ignore_network_failure
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    );

  auto&& addr
    ( net.connect_to
        ( fhg::com::host_t ( fhg::util::connectable_to_address_string
                               (net.local_endpoint().address())
                           )
        , fhg::com::port_t (std::to_string (net.local_endpoint().port()))
        )
    );

  while (n --> 0)
  {
    net.perform<sdpa::events::ErrorEvent>
      (addr, sdpa::events::ErrorEvent::SDPA_EUNKNOWN, "success");
  }

  counter.wait();
}
