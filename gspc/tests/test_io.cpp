#define BOOST_TEST_MODULE GspcNetIo
#include <boost/test/unit_test.hpp>

#include <gspc/net.hpp>
#include <gspc/net/io.hpp>

BOOST_AUTO_TEST_CASE (test_initialize_shutdown)
{
  gspc::net::initialize ();

  {
    gspc::net::server::queue_manager_t qmgr;
    gspc::net::server_ptr_t server =
      gspc::net::serve ("tcp://localhost:*", qmgr);
    BOOST_REQUIRE (server);

    server->stop ();
  }

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_many_initialize_shutdown)
{
  static const size_t NUM = 50;

  gspc::net::server::queue_manager_t qmgr;
  boost::system::error_code ec;
  int rc;

  for (size_t i = 0 ; i < NUM ; ++i)
  {
    gspc::net::initializer _net_init;

    gspc::net::server_ptr_t server =
      gspc::net::serve ("tcp://localhost:*", qmgr);

    BOOST_REQUIRE (server);

    gspc::net::client_ptr_t client =
      gspc::net::dial (server->url () + "?timeout=100", ec);

    BOOST_REQUIRE (client);

    while (not client->is_connected ())
    {
      rc = client->stop ();
      rc = client->start ();
    }

    rc = client->send ("/test", "");

    BOOST_REQUIRE_EQUAL (rc, 0);

    std::cerr << (i+1) << "/" << NUM << " ok\n";
  }
}
