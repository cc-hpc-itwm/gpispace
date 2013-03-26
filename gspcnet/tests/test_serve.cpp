#define BOOST_TEST_MODULE GspcNetServe
#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>

#include <gspc/net.hpp>
#include <gspc/net/error.hpp>
#include <gspc/net/serve.hpp>

#include <gspc/net/server.hpp>
#include <gspc/net/server/queue_manager.hpp>

namespace fs = boost::filesystem;

BOOST_AUTO_TEST_CASE (test_serve_tcp_socket_start_stop_loop)
{
  static const size_t NUM_ITERATIONS = 1000;

  gspc::net::server::queue_manager_t qmgr;

  for (size_t i = 0 ; i < NUM_ITERATIONS ; ++i)
  {
    gspc::net::server_ptr_t server =
      gspc::net::serve ("tcp://localhost:5000", qmgr);
    BOOST_REQUIRE (server);
  }
}

BOOST_AUTO_TEST_CASE (test_serve_unix_socket)
{
  static const size_t NUM_ITERATIONS = 1000;

  gspc::net::server::queue_manager_t qmgr;

  for (size_t i = 0 ; i < NUM_ITERATIONS ; ++i)
  {
    gspc::net::server_ptr_t server =
      gspc::net::serve ("unix://socket.foo", qmgr);
    BOOST_REQUIRE (server);

    BOOST_CHECK (fs::exists ("socket.foo"));
  }

  fs::remove ("socket.foo");
}
