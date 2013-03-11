#define BOOST_TEST_MODULE GspcNetBasics
#include <boost/test/unit_test.hpp>

#include <algorithm>

#include <gspc/net.hpp>

BOOST_AUTO_TEST_CASE (test_gspcnet_hello_world)
{
  int ec = 0;
  char buf [1024];
  size_t n;

  gspc::net::listener_t *listener = gspc::net::listen ("tcp://*:5000");

  // try to open a new connection
  gspc::net::connection_t *server = gspc::net::dial ("tcp://localhost:5000");

  // accept a new connection from a client
  gspc::net::connection_t *client = listener->accept (timeout);

  n = server->write ("hello world\n", 12, ec);
  BOOST_REQUIRE_EQUAL (ec, 0);
  BOOST_REQUIRE_EQUAL (n, 12);

  std::fill (&buf[0], &buf[0] + sizeof(buf), 0);

  n = client->read (buf, 12, ec);
  BOOST_REQUIRE_EQUAL (ec, 0);
  BOOST_REQUIRE_EQUAL (n, 12);

  delete client;
  delete server;
  delete listener;
}

struct EchoHandler
{
  int operator() ( gspc::net::request_t & rqst
                 , gspc::net::response_t & rply
                 )
  {
    rply.header = rqst.header;
    gspc::net::io::copy (rqst.body, reply.body);
  }
};

BOOST_AUTO_TEST_CASE (test_gspcnet_handle)
{
  gspc::net::initialize ();

  gspc::net::handle ("/echo/", EchoHandler ());
  gspc::net::spawn_listen_and_serve (":5000");

  gspc::net::finalize ();
}

BOOST_AUTO_TEST_CASE (test_gspcnet_handle_special)
{
  gspc::net::server_parameter_t p (":5000");
  p.handler = gspc::net::default_mux ();

  gspc::net::server_ptr server = gspc::net::new_server (p);
  server->spawn_listen_and_serve ();
}

BOOST_AUTO_TEST_CASE (test_gspcnet_client)
{
  gspc::net::handle ("/echo/", EchoHandler ());
  gspc::net::spawn_listen_and_serve (":5000");

  gspc::net::client_ptr client = gspc::net::connect
    ("gspc://localhost:5000?login=gspc&passcode=1234&timeout=1000");

  gspc::net::request_t request ("/echo/");
  request.header ["content-type"] = "text/plain";
  request.body = gspc::net::io::new_memory_reader ("Hello, World!", 14);

  gspc::net::response_t response;
  client->request (request, response);

  BOOST_REQUIRE_EQUAL
    (gspc::net::io::get_string (response->body), "Hello, World!");

  client->get ("/echo/");

  gspc::net::finalize ();
}

BOOST_AUTO_TEST_CASE (test_gspcnet_drts)
{
  // create a router service (using default lookup mechanism)
  gspc::net::handle ( "/drts/", RouterHandler ());
  gspc::net::handle ( "/drts/agent-A/", EchoHandler ());
  gspc::net::spawn_listen_and_serve (":5000");

  // sdpa messages:
  //    dst: <agent-B>
  //    src: <agent-A>
  //   body: encoded message

  gspc::net::response_ptr response =
    gspc::net::post ( "http://localhost:5000/drts/agent-B"
                    , gspc::net::content_type::text_plain ()
                    , gspc::net::io::memory_reader ("Hello, World!", 14)
                    );

  // request will be handled by RouterHandler:
  // already connected to 'agent-B'?
  //   if not:
  //      lookup url
  //      if not yet connected to url:
  //         create new client and connect it to the given url
  //         register error handler with client
  //
  //   send request to stored client
  //
  // if dst equals self:
  //   strip prefix and hand message to next handler
  //   reply with something meaningful

  BOOST_REQUIRE_EQUAL
    (gspc::net::io::get_string (response->body), "Hello, World!");

  gspc::net::finalize ();
}
