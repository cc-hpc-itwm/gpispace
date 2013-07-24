#define BOOST_TEST_MODULE GspcNetServe
#include <boost/test/unit_test.hpp>

#include <errno.h>
#include <string.h>
#include <sys/resource.h>

#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/system/system_error.hpp>

#include <gspc/net.hpp>
#include <gspc/net/io.hpp>
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
  static size_t max_open_files;

  SetRLimits ()
  {
    struct rlimit lim;
    lim.rlim_cur = lim.rlim_max = 60000;

    if (-1 == getrlimit (RLIMIT_NOFILE, &lim))
    {
      std::cerr << "getrlimit failed: " << strerror (errno) << std::endl;
    }

    lim.rlim_cur = lim.rlim_max;

    if (-1 == setrlimit (RLIMIT_NOFILE, &lim))
    {
      std::cerr << "setrlimit failed: " << strerror (errno) << std::endl;
    }

    max_open_files = (lim.rlim_cur == RLIM_INFINITY) ? 60000 : lim.rlim_cur;

    std::cerr << "max_open_files := " << max_open_files << std::endl;
  }

  ~SetRLimits ()
  {
  }
};
size_t SetRLimits::max_open_files = 0;

BOOST_GLOBAL_FIXTURE (SetRLimits);

BOOST_AUTO_TEST_CASE (test_serve_tcp_socket_start_stop_loop)
{
  gspc::net::initialize ();

  static const size_t NUM_ITERATIONS = 1000;

  gspc::net::server::queue_manager_t qmgr;

  for (size_t i = 0 ; i < NUM_ITERATIONS ; ++i)
  {
    gspc::net::server_ptr_t server =
      gspc::net::serve ("tcp://localhost:*", qmgr);
    BOOST_REQUIRE (server);
    server->stop ();
  }

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_serve_unix_socket)
{
  gspc::net::initialize ();

  static const size_t NUM_ITERATIONS = 1000;

  gspc::net::server::queue_manager_t qmgr;

  for (size_t i = 0 ; i < NUM_ITERATIONS ; ++i)
  {
    gspc::net::server_ptr_t server =
      gspc::net::serve ("unix://socket.foo", qmgr);
    BOOST_REQUIRE (server);

    BOOST_CHECK (fs::exists ("socket.foo"));

    server->stop ();
  }

  fs::remove ("socket.foo");

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_serve_unix_socket_connect)
{
  gspc::net::initialize ();

  gspc::net::server::queue_manager_t qmgr;

  gspc::net::server_ptr_t server =
    gspc::net::serve ("unix://socket.foo", qmgr);
  BOOST_REQUIRE (server);

  gspc::net::client_ptr_t client =
    gspc::net::dial (server->url ());

  BOOST_REQUIRE (client);

  client->stop ();
  server->stop ();

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_serve_unix_socket_connect_many)
{
  gspc::net::initialize ();

  using namespace gspc::net::tests;

  const size_t N_FD_PER_CLIENT = 2; // c_socket + s_socket
  const size_t N_FD_SERVER     = 1; // s_socket
  const size_t N_FD_BASE       = 3 + 3; // stdin, stdout, stderr, io_service

  static const size_t NUM_CLIENTS =
    (SetRLimits::max_open_files - (N_FD_SERVER + N_FD_BASE)) / N_FD_PER_CLIENT;

  //  static const size_t NUM_CLIENTS = 256;

  gspc::net::server::queue_manager_t qmgr;

  gspc::net::server_ptr_t server =
    //    gspc::net::serve ("tcp://localhost:*", qmgr);
    gspc::net::serve ("unix://socket.foo", qmgr);
  BOOST_REQUIRE (server);

  mock::user subscriber;
  qmgr.subscribe (&subscriber, "/test/send", "mock-1", gspc::net::frame ());

  std::vector<gspc::net::client_ptr_t> clients;

  std::size_t clients_created = 0;
  for (size_t i = 0 ; i < NUM_CLIENTS ; ++i)
  {
    using namespace boost::system;

    gspc::net::client_ptr_t client;

    boost::system::error_code ec;
    client = gspc::net::dial (server->url () + "?timeout=1000", ec);

    if (ec)
    {
      std::cerr << "could not connect: " << ec.message () << std::endl;
      break;
    }
    else
    {
      BOOST_REQUIRE (client);
    }

    clients.push_back (client);
    ++clients_created;

    client->send ("/test/send", "hello world");
  }

  while (subscriber.frames.size () != clients_created)
  {
    usleep (100);
  }

  BOOST_REQUIRE (clients_created > 0);
  BOOST_REQUIRE_EQUAL (clients.size (), clients_created);
  BOOST_REQUIRE_EQUAL (subscriber.frames.size (), clients_created);

  std::cerr << "simulated " << clients_created
            << " clients in one process"
            << std::endl;

  clients.clear ();

  server->stop ();

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_serve_tcp_socket_already_in_use)
{
  gspc::net::initialize ();

  gspc::net::server::queue_manager_t qmgr;

  gspc::net::server_ptr_t server =
    gspc::net::serve ("tcp://localhost:0", qmgr);
  BOOST_REQUIRE (server);

  BOOST_CHECK_THROW ( gspc::net::serve (server->url (), qmgr)
                    , boost::system::system_error
                    );
  server->stop ();

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_serve_unix_socket_connection_refused)
{
  gspc::net::initialize ();

  BOOST_CHECK_THROW ( gspc::net::dial ("unix://this.socket.does.not.exist")
                    , boost::system::system_error
                    );

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_serve_send_unix)
{
  gspc::net::initialize ();

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
  {
    BOOST_REQUIRE (0 == client->send ("/test/send", "hello world!"));
  }

  while (subscriber.frames.size () != NUM_MSGS_TO_SEND)
  {
    usleep (100);
  }

  BOOST_REQUIRE_EQUAL ( subscriber.frames.size ()
                      , NUM_MSGS_TO_SEND
                      );


  server->stop ();
  client->stop ();

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_serve_disconnected_client)
{
  gspc::net::initialize ();

  using namespace gspc::net::tests;

  int rc;
  gspc::net::server::queue_manager_t qmgr;
  mock::user subscriber;
  qmgr.subscribe (&subscriber, "/test/send", "mock-1", gspc::net::frame ());

  gspc::net::server_ptr_t server =
    gspc::net::serve ("tcp://localhost:*", qmgr);
  BOOST_REQUIRE (server);

  gspc::net::client_ptr_t client (gspc::net::dial (server->url ()));
  BOOST_REQUIRE (client);

  client->disconnect ();

  rc = client->send_sync ( "/test/send"
                         , "hello world!"
                         , boost::posix_time::pos_infin
                         );
  BOOST_REQUIRE_EQUAL (rc, gspc::net::E_UNAUTHORIZED);

  server->stop ();
  client->stop ();

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_serve_send_tcp)
{
  gspc::net::initialize ();

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


  client->stop ();
  server->stop ();

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_request_success)
{
  gspc::net::initialize ();

  static const std::size_t NUM_MSGS_TO_SEND = 10000;
  using namespace gspc::net::tests;

  gspc::net::server::service_demux_t demux;
  gspc::net::server::queue_manager_t qmgr (demux);

  demux.handle ("/service/echo-1", gspc::net::service::echo ());

  gspc::net::server_ptr_t server =
    gspc::net::serve ("tcp://localhost:*", qmgr);
  BOOST_REQUIRE (server);

  gspc::net::client_ptr_t client (gspc::net::dial (server->url ()));
  BOOST_REQUIRE (client);

  for (size_t i = 0 ; i < NUM_MSGS_TO_SEND ; ++i)
  {
    gspc::net::frame rply;
    int rc = client->request ( "/service/echo-1"
                             , "hello world!"
                             , rply
                             , boost::posix_time::seconds (1)
                             );
    BOOST_REQUIRE_EQUAL (rc, 0);
    BOOST_REQUIRE_EQUAL (rply.get_body_as_string (), "hello world!");
  }

  client->stop ();
  server->stop ();

  gspc::net::shutdown ();
}

BOOST_AUTO_TEST_CASE (test_request_no_such_service)
{
  gspc::net::initialize ();

  using namespace gspc::net::tests;

  gspc::net::server::queue_manager_t qmgr;

  gspc::net::server_ptr_t server =
    gspc::net::serve ("tcp://localhost:*", qmgr);
  BOOST_REQUIRE (server);

  gspc::net::client_ptr_t client (gspc::net::dial (server->url ()));
  BOOST_REQUIRE (client);

  gspc::net::frame rply;
  int rc = client->request ( "/service/unknown"
                           , "hello world!"
                           , rply
                           , boost::posix_time::seconds (1)
                           );
  BOOST_REQUIRE_EQUAL (rc, 0);
  BOOST_REQUIRE_EQUAL (rply.get_command (), "ERROR");
  BOOST_REQUIRE (rply.has_header ("code"));
  BOOST_REQUIRE_EQUAL (*rply.get_header ("code"), "404");
  BOOST_REQUIRE (rply.has_header ("message"));
  BOOST_REQUIRE_EQUAL (*rply.get_header ("message"), "no such service");

  client->stop ();
  server->stop ();

  gspc::net::shutdown ();
}
