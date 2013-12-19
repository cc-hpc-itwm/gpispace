#define BOOST_TEST_MODULE NetworkStrategyTest
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhglog/minimal.hpp>
#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <seda/Stage.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

static const std::string &kvs_host () { static std::string s("localhost"); return s; }
static const std::string &kvs_port () { static std::string s("1234"); return s; }

struct F
{
  F ()
    : m_pool (0)
    , m_kvsd (0)
    , m_serv (0)
    , m_thrd (0)
  {
    FHGLOG_SETUP();

    m_pool = new fhg::com::io_service_pool(1);
    m_kvsd = new fhg::com::kvs::server::kvsd;
    m_serv = new fhg::com::tcp_server ( *m_pool
                                      , *m_kvsd
                                      , kvs_host ()
                                      , kvs_port ()
                                      , true
                                      );
    m_thrd = new boost::thread (boost::bind ( &fhg::com::io_service_pool::run
                                            , m_pool
                                            )
                               );

    m_serv->start();

    fhg::com::kvs::get_or_create_global_kvs ( kvs_host()
                                            , kvs_port()
                                            , true
                                            , boost::posix_time::seconds(3)
                                            , 1
                                            );
  }

  ~F ()
  {
    m_serv->stop ();
    m_pool->stop ();
    m_thrd->join ();
    delete m_thrd;
    delete m_serv;
    delete m_kvsd;
    delete m_pool;
  }

  fhg::com::io_service_pool *m_pool;
  fhg::com::kvs::server::kvsd *m_kvsd;
  fhg::com::tcp_server *m_serv;
  boost::thread *m_thrd;
};

namespace
{
  struct wait_for_n_events_strategy
  {
    wait_for_n_events_strategy (unsigned int expected)
      : _counter (0)
      , _expected (expected)
    {}

    void perform (const boost::shared_ptr<seda::IEvent>&)
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

BOOST_FIXTURE_TEST_SUITE( s, F )

BOOST_AUTO_TEST_CASE ( perform_test )
{
  wait_for_n_events_strategy counter (1);

  seda::Stage<seda::IEvent> final
    (boost::bind (&wait_for_n_events_strategy::perform, &counter, _1));

  sdpa::com::NetworkStrategy net ( boost::bind (&seda::Stage<seda::IEvent>::send, &final, _1)
                                 , "peer-1"
                                 , fhg::com::host_t ("localhost")
                                 , fhg::com::port_t ("0")
                                 );

  net.perform (boost::shared_ptr<seda::IEvent>(new sdpa::events::ErrorEvent( "peer-1"
                                                              , "peer-1"
                                                              , sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                                                              , "success"
                                                              )
                                 )
               );

  counter.wait();
}

BOOST_AUTO_TEST_SUITE_END()
