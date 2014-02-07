#define BOOST_TEST_MODULE GspcNetIo
#include <boost/test/unit_test.hpp>

#include <gspc/net/client.hpp>
#include <gspc/net/dial.hpp>
#include <gspc/net/io.hpp>
#include <gspc/net/serve.hpp>
#include <gspc/net/server.hpp>
#include <gspc/net/server/service_demux.hpp>
#include <gspc/net/server/queue_manager.hpp>

#include "mock_user.hpp"

#include <iostream>

BOOST_AUTO_TEST_CASE (test_initialize_shutdown)
{
  gspc::net::initializer _net_init;

  gspc::net::server::service_demux_t service_demux;
  gspc::net::server::queue_manager_t qmgr (service_demux);
  gspc::net::server_ptr_t server =
    gspc::net::serve ("tcp://localhost:*", qmgr);
  BOOST_REQUIRE (server);

  server->stop ();
}

BOOST_AUTO_TEST_CASE (test_many_initialize_shutdown)
{
  static const size_t NUM = 25;

  gspc::net::server::service_demux_t service_demux;
  gspc::net::server::queue_manager_t qmgr (service_demux);
  gspc::net::tests::mock::user subscriber;

  qmgr.subscribe (&subscriber, "/test", "mock-1", gspc::net::frame ());

  int rc;

  for (size_t i = 0 ; i < NUM ; ++i)
  {
    gspc::net::initializer _net_init;

    gspc::net::server_ptr_t server =
      gspc::net::serve ("tcp://localhost:*", qmgr);

    BOOST_REQUIRE (server);

    gspc::net::client_ptr_t client
      (gspc::net::dial (server->url () + "?connect_timeout=100"));

    BOOST_REQUIRE (client);

    while (not client->is_connected ())
    {
      rc = client->stop ();
      BOOST_REQUIRE_EQUAL (rc, 0);
      rc = client->start ();

      if (0 == rc)
      {
        break;
      }
      else if (-ETIME == rc)
      {
        continue;
      }
      else if (-ENOTCONN)
      {
        continue;
      }

      std::cerr << "failed: " << strerror (-rc) << std::endl;
      BOOST_REQUIRE_NE (rc, 0);
    }

    rc = client->send_sync ("/test", "", boost::posix_time::seconds (3));
    BOOST_REQUIRE_EQUAL (rc, 0);

    std::cerr << (i+1) << "/" << NUM << " ok\n";
  }

  BOOST_REQUIRE_EQUAL (subscriber.frames.size (), NUM);
}
