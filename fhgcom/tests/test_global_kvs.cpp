#define BOOST_TEST_MODULE GlobalKVSTest
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("0"); return s; }

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
    m_kvsd = new fhg::com::kvs::server::kvsd ("");
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

    LOG(INFO, "kvs daemon is listening on port " << m_serv->port ());

    fhg::com::kvs::global::get_kvs_info().init( kvs_host()
                                              , boost::lexical_cast<std::string>(m_serv->port())
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

BOOST_AUTO_TEST_CASE ( put_get_loop_test )
{
  static const std::size_t NUM_ITERATIONS = (1 << 16);

  using namespace fhg::com;

  MLOG (INFO, "entering put/get loop (iterations := " << NUM_ITERATIONS << ")");

  for (std::size_t i = 0 ; i < NUM_ITERATIONS ; ++i)
  {
    kvs::put ("test_global_kvs", i);
    std::size_t val = kvs::get<std::size_t> ("test_global_kvs");

    BOOST_CHECK_EQUAL (val, i);
  }
}

BOOST_AUTO_TEST_CASE ( put_get_int_test )
{
  using namespace fhg::com;

  kvs::put ("test_global_int_kvs", 42);

  int val (kvs::get<int> ("test_global_int_kvs"));
  BOOST_CHECK_EQUAL (val, 42);

  kvs::del ("test_global_int_kvs");
}

BOOST_AUTO_TEST_CASE ( put_get_timed_valid_test )
{
  using namespace fhg::com;

  int val;

  kvs::timed_put ("test", 42, 5000); // 5 seconds
  val = kvs::get<int> ("test");
  BOOST_CHECK_EQUAL (val, 42);
  kvs::del ("test");
}

BOOST_AUTO_TEST_CASE ( put_get_timed_expired_test )
{
  using namespace fhg::com;

  kvs::timed_put ("test", 42, 10); // 10 ms
  usleep (50);

  try
  {
    kvs::get<int> ("test");
    kvs::del ("test");
  }
  catch (...)
  {
    // ok
  }
}

BOOST_AUTO_TEST_SUITE_END()

// outside of suite, we don't want the kvsd
BOOST_AUTO_TEST_CASE ( no_server_test )
{
  using namespace fhg::com;

  const std::string host ("localhost");
  const std::string port ("1234");

  try
  {
    kvs::global::get_kvs_info().init
      ( host
      , port
      , boost::posix_time::seconds(1)
      , 1
      );
    kvs::put ("test_global_int_kvs", 42);
    BOOST_CHECK(false);
  }
  catch (...)
  {
    // ok
  }
}

BOOST_AUTO_TEST_CASE (boost_ptime_comparison)
{
  using namespace boost::posix_time;

  ptime def (min_date_time);
  ptime now = microsec_clock::universal_time ();
  ptime exp = now + microsec (500);

  BOOST_REQUIRE (now > def);
  BOOST_REQUIRE (now < exp);
  BOOST_REQUIRE (def == min_date_time);
}
