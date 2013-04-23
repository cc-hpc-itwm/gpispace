#define BOOST_TEST_MODULE GspcNetServe
#include <boost/test/unit_test.hpp>

#include <errno.h>
#include <string.h>
#include <sys/resource.h>

#include <boost/filesystem.hpp>
#include <boost/system/system_error.hpp>

#include <gspc/net.hpp>
#include <gspc/net/error.hpp>
#include <gspc/net/serve.hpp>
#include <gspc/net/dial.hpp>

#include <gspc/net/service/echo.hpp>

#include <gspc/net/server.hpp>
#include <gspc/net/server/queue_manager.hpp>
#include <gspc/net/server/service_demux.hpp>

#include "mock_user.hpp"

namespace fs = boost::filesystem;

struct SetRLimits
{
  SetRLimits ()
  {
    struct rlimit lim;
    lim.rlim_cur = 60000;
    lim.rlim_max = 60000;

    if (-1 == setrlimit (RLIMIT_NOFILE, &lim))
    {
      std::cerr << "setrlimit failed: " << strerror (errno) << std::endl;
    }
  }

  ~SetRLimits ()
  {}
};

BOOST_GLOBAL_FIXTURE (SetRLimits);

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
  using namespace gspc::net::tests;
  static const size_t NUM_CLIENTS = 10000;

  gspc::net::server::queue_manager_t qmgr;

  gspc::net::server_ptr_t server =
    gspc::net::serve ("unix://socket.foo", qmgr);
  BOOST_REQUIRE (server);

  mock::user subscriber;
  qmgr.subscribe (&subscriber, "/test/send", "mock-1", gspc::net::frame ());

  std::vector<gspc::net::client_ptr_t> clients;

  std::size_t msgs_sent = 0;
  for (size_t i = 0 ; i < NUM_CLIENTS ; ++i)
  {
    gspc::net::client_ptr_t client =
      gspc::net::dial (server->url ());
    BOOST_CHECK (client);

    if (client)
    {
      clients.push_back (client);

      client->send ("/test/send", "hello world");
      ++msgs_sent;
    }
  }

  BOOST_REQUIRE_EQUAL (clients.size (), NUM_CLIENTS);

  while (subscriber.frames.size () != msgs_sent)
  {
    usleep (100);
  }

  BOOST_REQUIRE_EQUAL (subscriber.frames.size (), msgs_sent);
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
  BOOST_CHECK_THROW ( gspc::net::dial ("unix://this.socket.does.not.exist")
                    , boost::system::system_error
                    );
}

BOOST_AUTO_TEST_CASE (test_serve_send_unix)
{
  static const std::size_t NUM_MSGS_TO_SEND = 10000;
  using namespace gspc::net::tests;

  gspc::net::server::queue_manager_t qmgr;
  mock::user subscriber;
  qmgr.subscribe (&subscriber, "/test/send", "mock-1", gspc::net::frame ());

  gspc::net::server_ptr_t server =
    gspc::net::serve ("unix://socket.foo", qmgr);
  BOOST_REQUIRE (server);

  gspc::net::client_ptr_t client (gspc::net::dial (server->url ()));
  BOOST_REQUIRE (client);

  for (size_t i = 0 ; i < NUM_MSGS_TO_SEND ; ++i)
    client->send ("/test/send", "hello world!");

  while (subscriber.frames.size () != NUM_MSGS_TO_SEND)
  {
    usleep (100);
  }

  BOOST_REQUIRE_EQUAL ( subscriber.frames.size ()
                      , NUM_MSGS_TO_SEND
                      );
}

BOOST_AUTO_TEST_CASE (test_serve_disconnected_client)
{
  using namespace gspc::net::tests;

  gspc::net::server::queue_manager_t qmgr;
  mock::user subscriber;
  qmgr.subscribe (&subscriber, "/test/send", "mock-1", gspc::net::frame ());

  gspc::net::server_ptr_t server =
    gspc::net::serve ("tcp://localhost:*", qmgr);
  BOOST_REQUIRE (server);

  gspc::net::client_ptr_t client (gspc::net::dial (server->url ()));
  BOOST_REQUIRE (client);

  client->disconnect ();

  BOOST_REQUIRE_EQUAL ( client->send ("/test/send", "hello world!")
                      , gspc::net::E_UNAUTHORIZED
                      );
}

BOOST_AUTO_TEST_CASE (test_serve_send_tcp)
{
  static const std::size_t NUM_MSGS_TO_SEND = 10000;
  using namespace gspc::net::tests;

  gspc::net::server::queue_manager_t qmgr;
  mock::user subscriber;
  qmgr.subscribe (&subscriber, "/test/send", "mock-1", gspc::net::frame ());

  gspc::net::server_ptr_t server =
    gspc::net::serve ("tcp://localhost:*", qmgr);
  BOOST_REQUIRE (server);

  gspc::net::client_ptr_t client (gspc::net::dial (server->url ()));
  BOOST_REQUIRE (client);

  for (size_t i = 0 ; i < NUM_MSGS_TO_SEND ; ++i)
    client->send ("/test/send", "hello world!");

  while (subscriber.frames.size () != NUM_MSGS_TO_SEND)
  {
    usleep (100);
  }

  BOOST_REQUIRE_EQUAL ( subscriber.frames.size ()
                      , NUM_MSGS_TO_SEND
                      );
}

BOOST_AUTO_TEST_CASE (test_request_success)
{
  static const std::size_t NUM_MSGS_TO_SEND = 10000;
  using namespace gspc::net::tests;

  gspc::net::server::service_demux_t demux;
  gspc::net::server::queue_manager_t qmgr (demux);

  demux.handle ("/test/echo-1", gspc::net::service::echo ());

  gspc::net::server_ptr_t server =
    gspc::net::serve ("tcp://localhost:*", qmgr);
  BOOST_REQUIRE (server);

  gspc::net::client_ptr_t client (gspc::net::dial (server->url ()));
  BOOST_REQUIRE (client);

  for (size_t i = 0 ; i < NUM_MSGS_TO_SEND ; ++i)
  {
    gspc::net::frame rply;
    int rc = client->request ( "/test/echo-1"
                             , "hello world!"
                             , rply
                             );
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE_EQUAL (rply.get_body_as_string (), "hello world!");
  }
}

BOOST_AUTO_TEST_CASE (test_request_no_such_service)
{
  using namespace gspc::net::tests;

  gspc::net::server::queue_manager_t qmgr;

  gspc::net::server_ptr_t server =
    gspc::net::serve ("tcp://localhost:*", qmgr);
  BOOST_REQUIRE (server);

  gspc::net::client_ptr_t client (gspc::net::dial (server->url ()));
  BOOST_REQUIRE (client);

  gspc::net::frame rply;
  int rc = client->request ( "/test/echo-1"
                           , "hello world!"
                           , rply
                           );
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (rply.get_command (), "ERROR");
  BOOST_REQUIRE (rply.has_header ("code"));
  BOOST_REQUIRE_EQUAL (*rply.get_header ("code"), "404");
  BOOST_REQUIRE (rply.has_header ("message"));
  BOOST_REQUIRE_EQUAL (*rply.get_header ("message"), "no such service");
}
