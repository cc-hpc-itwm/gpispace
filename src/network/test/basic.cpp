// bernd.loerwald@itwm.fraunhofer.de
#define BOOST_TEST_MODULE network

#include <network/client.hpp>
#include <network/connection.hpp>
#include <network/server.hpp>

#include <fhg/util/boost/test/printer/vector.hpp>
#include <fhg/util/random_string.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/scoped_thread.hpp>

//! \todo more elaborate test cases, split by component

namespace
{
  fhg::network::buffer_type id (fhg::network::buffer_type buffer)
  {
    return buffer;
  }
}

BOOST_AUTO_TEST_CASE (continous_acceptor_properly_stops_and_endpoint_is_reusable)
{
  boost::asio::ip::tcp::endpoint endpoint (boost::asio::ip::address(), 0);

  boost::asio::io_service io_service;

  {
    fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor
      ( endpoint
      , io_service
      , id
      , id
      , [] (fhg::network::connection_type*, fhg::network::buffer_type) {}
      , [] (fhg::network::connection_type*) {}
      , [] (std::unique_ptr<fhg::network::connection_type>) {}
      );
  }
  {
    fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor
      ( endpoint
      , io_service
      , id
      , id
      , [] (fhg::network::connection_type*, fhg::network::buffer_type) {}
      , [] (fhg::network::connection_type*) {}
      , [] (std::unique_ptr<fhg::network::connection_type>) {}
      );
  }
}

BOOST_AUTO_TEST_CASE (connecting_sending_and_disconnecting_triggers_handlers)
{
  boost::asio::ip::tcp::endpoint const endpoint (boost::asio::ip::address(), 0);

  fhg::util::thread::event<std::unique_ptr<fhg::network::connection_type>> serverside_connected;
  std::unique_ptr<fhg::network::connection_type> serverside_connection;

  bool client_got_disconnect (false);
  fhg::util::thread::event<fhg::network::connection_type*> server_got_disconnect;

  fhg::util::thread::event<fhg::network::buffer_type> buffer_received_client;
  fhg::util::thread::event<fhg::network::buffer_type> buffer_received_server;

  boost::asio::io_service io_service_server;

  fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor
    ( endpoint
    , io_service_server
    , id
    , id
    , [&] (fhg::network::connection_type*, fhg::network::buffer_type buffer)
    {
      buffer_received_server.notify (buffer);
    }
    , [&] (fhg::network::connection_type* connection)
    {
      server_got_disconnect.notify (connection);
    }
    , [&] (std::unique_ptr<fhg::network::connection_type> connection)
    {
      serverside_connected.notify (std::move (connection));
    }
    );

  const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
    io_service_thread_server ([&io_service_server] { io_service_server.run(); });

  {
    boost::asio::io_service io_service_client;

    boost::asio::io_service::work io_service_work_client (io_service_client);
    const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      io_service_thread_client ([&io_service_client] { io_service_client.run(); });

    std::unique_ptr<fhg::network::connection_type> clientside_connection
      ( fhg::network::connect_client<boost::asio::ip::tcp>
        ( io_service_client
        , acceptor.local_endpoint()
        , id
        , id
        , [&] (fhg::network::buffer_type buffer)
        {
          buffer_received_client.notify (buffer);
        }
        , [&] (fhg::network::connection_type*)
        {
          client_got_disconnect = true;
        }
        )
      );

    BOOST_REQUIRE (clientside_connection);
    std::unique_ptr<fhg::network::connection_type> tmp (serverside_connected.wait());
    BOOST_REQUIRE (tmp);
    std::swap (serverside_connection, tmp);
    BOOST_REQUIRE (serverside_connection);

    {
      std::string string (fhg::util::random_string());
      fhg::network::buffer_type const buffer (string.begin(), string.end());

      clientside_connection->send (buffer);

      BOOST_REQUIRE_EQUAL (buffer, buffer_received_server.wait());
    }

    {
      std::string string (fhg::util::random_string());
      fhg::network::buffer_type const buffer (string.begin(), string.end());

      serverside_connection->send (buffer);

      BOOST_REQUIRE_EQUAL (buffer, buffer_received_client.wait());
    }

    io_service_client.stop();
  }

  BOOST_REQUIRE_EQUAL (server_got_disconnect.wait(), serverside_connection.get());
  serverside_connection.reset();

  BOOST_REQUIRE (!client_got_disconnect);

  io_service_server.stop();
}
