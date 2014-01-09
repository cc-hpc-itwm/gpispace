#ifndef SDPA_TESTS_KVS_SETUP_FIXTURE
#define SDPA_TESTS_KVS_SETUP_FIXTURE

#include <fhglog/fhglog.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

#include <boost/thread.hpp>

static std::string current_kvs_port = "0";
static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string & kvs_port () { return current_kvs_port; }

struct KVSSetup
{
  KVSSetup ()
    : m_pool (1)
    , m_kvsd ("")
    , m_serv (m_pool, m_kvsd, kvs_host(), kvs_port(), true)
    , m_thrd (boost::bind (&fhg::com::io_service_pool::run, &m_pool))
  {
	  setenv("FHGLOG_level", "TRACE", true);
	  FHGLOG_SETUP();

	  m_serv.start();

	  current_kvs_port = boost::lexical_cast<std::string>(m_serv.port());

	  fhg::com::kvs::global::get_kvs_info().init( kvs_host()
                                              	  , kvs_port()
                                              	  , boost::posix_time::seconds(10)
                                              	  , 3
                                              	  );

  }

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
};

#endif
