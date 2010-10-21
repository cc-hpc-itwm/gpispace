#define BOOST_TEST_MODULE GlobalKVSTest
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

#include <boost/thread.hpp>

BOOST_AUTO_TEST_CASE ( put_get_test )
{
  FHGLOG_SETUP();

  using namespace fhg::com;

  const std::string host ("localhost");
  const std::string port ("1234");

  io_service_pool pool (1);
  kvs::server::kvsd kvsd ("");
  tcp_server server ( pool
                    , kvsd
                    , host
                    , port
                    , true
                    );

  server.start ();
  boost::thread thrd (boost::bind ( &fhg::com::io_service_pool::run, &pool ));

  kvs::get_or_create_global_kvs (host, port);
  kvs::put ("test_global_kvs", 42);
  std::string val (kvs::get ("test_global_kvs").begin()->second);

  BOOST_CHECK_EQUAL (val, "42");

  pool.stop();
  thrd.join();
}

BOOST_AUTO_TEST_CASE ( put_get_int_test )
{
  using namespace fhg::com;

  const std::string host ("localhost");
  const std::string port ("1234");

  io_service_pool pool (1);
  kvs::server::kvsd kvsd ("");
  tcp_server server ( pool
                    , kvsd
                    , host
                    , port
                    , true
                    );

  server.start ();
  boost::thread thrd (boost::bind ( &fhg::com::io_service_pool::run, &pool ));

  kvs::get_or_create_global_kvs (host, port);
  kvs::put ("test_global_int_kvs", 42);
  int val (kvs::get<int> ("test_global_int_kvs"));
  BOOST_CHECK_EQUAL (val, 42);

  pool.stop();
  thrd.join();
}
