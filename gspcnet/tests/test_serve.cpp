#define BOOST_TEST_MODULE GspcNetServe
#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>

#include <gspc/net.hpp>
#include <gspc/net/error.hpp>
#include <gspc/net/serve.hpp>

#include <gspc/net/server.hpp>
#include <gspc/net/server/queue_manager.hpp>

namespace fs = boost::filesystem;

BOOST_AUTO_TEST_CASE (test_serve_tcp_socket)
{
  gspc::net::server::queue_manager_t qmgr;

  gspc::net::server_t *server =
    gspc::net::serve ("tcp://localhost:5000", qmgr);

  BOOST_REQUIRE (server);

  server->start ();

  BOOST_REQUIRE (fs::exists ("test-server"));

  server->stop ();

  BOOST_REQUIRE (not fs::exists ("test-server"));

  delete server;
}
