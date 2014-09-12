#define BOOST_TEST_MODULE NetworkStrategyTest
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhglog/LogMacros.hpp>
#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <boost/thread.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

#include <functional>

struct KVSSetup
{
  KVSSetup ()
    : m_pool (1)
    , m_kvsd()
    , m_serv (m_pool.get_io_service(), m_kvsd, "localhost", "0", true)
    , m_thrd (&fhg::com::io_service_pool::run, &m_pool)
    , _kvs ( new fhg::com::kvs::client::kvsc
             ( "localhost"
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
    m_pool.stop ();
    m_thrd.join ();
  }

  fhg::com::io_service_pool m_pool;
  fhg::com::kvs::server::kvsd m_kvsd;
  fhg::com::tcp_server m_serv;
  boost::thread m_thrd;
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

    void perform (const boost::shared_ptr<sdpa::events::SDPAEvent>&)
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

  sdpa::com::NetworkStrategy net
    ( std::bind
      (&wait_for_n_events_strategy::perform, &counter, std::placeholders::_1)
    , "peer-1"
    , fhg::com::host_t ("localhost")
    , fhg::com::port_t ("0")
    , _kvs
    );

  net.perform (boost::shared_ptr<sdpa::events::SDPAEvent>(new sdpa::events::ErrorEvent( "peer-1"
                                                              , "peer-1"
                                                              , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                                                              , "success"
                                                              )
                                 )
               );

  counter.wait();
}
