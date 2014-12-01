// bernd.loerwald@itwm.fraunhofer.de

#include <network/client.hpp>
#include <network/connection.hpp>
#include <network/server.hpp>

#include <fhg/util/daemonize.hpp>
#include <fhg/util/measure_average_time.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/scoped_thread.hpp>

namespace
{
  fhg::network::buffer_type id (fhg::network::buffer_type buffer)
  {
    return buffer;
  }

  void run_server()
  {
    fhg::util::thread::event<std::unique_ptr<fhg::network::connection_type>> serverside_connected;
    std::unique_ptr<fhg::network::connection_type> serverside_connection;

    fhg::util::thread::event<fhg::network::connection_type*> server_got_disconnect;

    boost::asio::io_service io_service;

    fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor
      ( boost::asio::ip::tcp::endpoint (boost::asio::ip::address(), 0)
      , io_service
      , id
      , id
      , [&] ( fhg::network::connection_type* connection
            , fhg::network::buffer_type buffer
            )
      {
        connection->send (buffer);
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

    std::cout << acceptor.local_endpoint().port() << "\n";

    boost::optional<pid_t> child_pid (fhg::util::fork_and_daemonize_child());

    if (child_pid)
    {
      std::cout << child_pid.get() << "\n";
    }
    else
    {
      const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
        io_service_thread ([&io_service] { io_service.run(); });

      std::unique_ptr<fhg::network::connection_type> tmp
        (serverside_connected.wait());
      std::swap (serverside_connection, tmp);

      server_got_disconnect.wait();

      io_service.stop();
    }
  }

  void run_client (std::string host, unsigned short port, std::size_t payload_size)
  {
    boost::asio::io_service io_service;

    fhg::util::thread::event<fhg::network::buffer_type> buffer_received;

    boost::asio::io_service::work io_service_work (io_service);
    const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
      io_service_thread ([&io_service] { io_service.run(); });

    std::unique_ptr<fhg::network::connection_type> clientside_connection
      ( fhg::network::connect_client<boost::asio::ip::tcp>
        ( io_service
        , host
        , port
        , id
        , id
        , [&] (fhg::network::buffer_type buffer)
        {
          buffer_received.notify (buffer);
        }
        , [&] (fhg::network::connection_type*) {}
        )
      );

    fhg::network::buffer_type const buffer (payload_size, 'X');

    std::cout << fhg::util::measure_average_time<std::chrono::microseconds>
                  ( [&]
                    {
                      clientside_connection->send (buffer);
                      buffer_received.wait();
                    }
                  , 500
                  ).count()
             << "µs\n";

    io_service.stop();
  }
}

// ssh $HOST_A ~/multiple_binaries_binary
//               $HOST_B $(ssh $HOST_B ~/multiple_binaries_binary | head -n1)
//               $((32*2**10))
//
// server: ./$0
// client: ./$0 host port payload_size

int main (int argc, char** argv)
try
{
  if (argc == 4)
  {
    run_client ( argv[1]
               , boost::lexical_cast<unsigned short> (argv[2])
               , boost::lexical_cast<std::size_t> (argv[3])
               );
  }
  else
  {
    run_server();
  }
  return 0;
}
catch (std::runtime_error const& ex)
{
  std::cerr << ex.what() << std::endl;
  return 1;
}
