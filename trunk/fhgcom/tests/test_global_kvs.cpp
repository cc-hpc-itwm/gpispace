#define BOOST_TEST_MODULE GlobalKVSTest
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

#include <boost/thread.hpp>

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("1234"); return s; }

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
    m_kvsd = new fhg::com::kvs::server::kvsd ("/tmp/notthere");
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
    sleep (1);

    fhg::com::kvs::get_or_create_global_kvs ( kvs_host()
                                            , kvs_port()
                                            , true
                                            , boost::posix_time::seconds(10)
                                            , 3
                                            );
  }

  ~F ()
  {
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

BOOST_FIXTURE_TEST_SUITE( s, F )

BOOST_AUTO_TEST_CASE ( put_get_test )
{
  using namespace fhg::com;

  kvs::put ("test_global_kvs", 42);
  std::string val (kvs::get<std::string> ("test_global_kvs"));

  BOOST_CHECK_EQUAL (val, "42");

  kvs::del ("test_global_kvs");
}

BOOST_AUTO_TEST_CASE ( put_get_int_test )
{
  using namespace fhg::com;

  kvs::put ("test_global_int_kvs", 42);

  int val (kvs::get<int> ("test_global_int_kvs"));
  BOOST_CHECK_EQUAL (val, 42);

  kvs::del ("test_global_int_kvs");
}

BOOST_AUTO_TEST_SUITE_END()


// outside of suite, we don't want the kvsd
BOOST_AUTO_TEST_CASE ( no_server_test )
{
  using namespace fhg::com;

  const std::string host ("localhost");
  const std::string port ("1234");

  kvs::get_or_create_global_kvs (host, port, true, boost::posix_time::seconds(10), 3);
  try
  {
    kvs::put ("test_global_int_kvs", 42);
    BOOST_CHECK(false);
  }
  catch (...)
  {
    // ok
  }
}
