#define BOOST_TEST_MODULE NetworkStrategyTest
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhglog/LogMacros.hpp>
#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <boost/thread.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/tcp_server.hpp>

#include <boost/asio/io_service.hpp>

#include <functional>

struct KVSSetup
{
  KVSSetup ()
    : _io_service()
    , _io_service_work (_io_service)
    , m_kvsd()
    , m_serv (_io_service, m_kvsd, "localhost", "0", true)
    , _io_service_thread ([this] { _io_service.run(); })
    , _kvs ( new fhg::com::kvs::client::kvsc
             ( _kvs_client_io_service
             , "localhost"
             , boost::lexical_cast<std::string> (m_serv.port())
             , true // auto_reconnect
             , boost::posix_time::seconds (3)
             , 3
             )
           )
  {}

  ~KVSSetup ()
  {
    m_serv.stop ();
    _io_service.stop();
    _io_service_thread.join();
  }

  boost::asio::io_service _io_service;
  boost::asio::io_service::work _io_service_work;
  fhg::com::kvs::server::kvsd m_kvsd;
  fhg::com::tcp_server m_serv;
  boost::thread _io_service_thread;
  boost::asio::io_service _kvs_client_io_service;
  fhg::com::kvs::kvsc_ptr_t _kvs;
};

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

      while (_counter < _expected)
      {
        _expected_count_reached.wait (_);
      }

      BOOST_REQUIRE_EQUAL (_counter, _expected);
    }

    mutable boost::mutex _counter_mutex;
    mutable boost::condition_variable _expected_count_reached;
    unsigned int _counter;
    unsigned int _expected;
  };
}

BOOST_FIXTURE_TEST_CASE (perform_test, KVSSetup)
{
  wait_for_n_events_strategy counter (1);

  boost::asio::io_service peer_io_service;
  sdpa::com::NetworkStrategy net
    ( std::bind ( &wait_for_n_events_strategy::perform
                , &counter
                , std::placeholders::_1
                , std::placeholders::_2
                )
    , peer_io_service
    , "peer-1"
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    , _kvs
    );

  net.perform ( net.connect_to_via_kvs ("peer-1")
              , boost::shared_ptr<sdpa::events::SDPAEvent>(new sdpa::events::ErrorEvent(sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                                                              , "success"
                                                              )
                                 )
               );

  counter.wait();
}
