#define BOOST_TEST_MODULE GspcNetServe
#include <boost/test/unit_test.hpp>

#include <boost/filesystem.hpp>
#include <boost/system/system_error.hpp>

#include <gspc/net.hpp>
#include <gspc/net/error.hpp>
#include <gspc/net/serve.hpp>
#include <gspc/net/dial.hpp>

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
      gspc::net::serve ("tcp://localhost:*", qmgr);
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

BOOST_AUTO_TEST_CASE (test_serve_unix_socket_connect)
{
  gspc::net::server::queue_manager_t qmgr;

  gspc::net::server_ptr_t server =
    gspc::net::serve ("unix://socket.foo", qmgr);
  BOOST_REQUIRE (server);

  gspc::net::client_ptr_t client =
    gspc::net::dial (server->url ());

  BOOST_REQUIRE (client);
}

BOOST_AUTO_TEST_CASE (test_serve_unix_socket_connect_many)
{
  // one might have to increase ulimit nofile for that to work
  static const size_t NUM_CLIENTS = 1000;

  gspc::net::server::queue_manager_t qmgr;

  gspc::net::server_ptr_t server =
    gspc::net::serve ("unix://socket.foo", qmgr);
  BOOST_REQUIRE (server);

  std::vector<gspc::net::client_ptr_t> clients;

  for (size_t i = 0 ; i < NUM_CLIENTS ; ++i)
  {
    gspc::net::client_ptr_t client =
      gspc::net::dial (server->url ());
    BOOST_CHECK (client);

    if (client)
      clients.push_back (client);
  }

  BOOST_REQUIRE_EQUAL (clients.size (), NUM_CLIENTS);
}

BOOST_AUTO_TEST_CASE (test_serve_tcp_socket_already_in_use)
{
  gspc::net::server::queue_manager_t qmgr;

  gspc::net::server_ptr_t server =
    gspc::net::serve ("tcp://localhost:0", qmgr);
  BOOST_REQUIRE (server);

  BOOST_CHECK_THROW ( gspc::net::serve (server->url (), qmgr)
                    , boost::system::system_error
                    );
}

BOOST_AUTO_TEST_CASE (test_serve_unix_socket_connection_refused)
{
  BOOST_CHECK_THROW ( gspc::net::dial ("unix://socket.foo")
                    , boost::system::system_error
                    );
}
