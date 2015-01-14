// bernd.loerwald@itwm.fraunhofer.de

#include <rif/entry_point.hpp>

#include <fhg/syscall.hpp>
#include <fhg/util/boost/asio/ip/address.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/boost/serialization/path.hpp>
#include <fhg/util/boost/serialization/unordered_map.hpp>
#include <fhg/util/print_exception.hpp>

#include <network/server.hpp>

#include <rif/execute_and_get_startup_messages.hpp>

#include <rpc/server.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/scoped_thread.hpp>

namespace
{
  namespace option
  {
    constexpr const char* const port {"port"};
    constexpr const char* const register_host {"register-host"};
    constexpr const char* const register_port {"register-port"};
  }
}

int main (int argc, char** argv)
try
{
  boost::program_options::options_description options_description;
  options_description.add_options()
    ( option::port
    , boost::program_options::value
        <fhg::util::boost::program_options::positive_integral<unsigned short>>()
    , "port to listen on"
    )
    ( option::register_host
    , boost::program_options::value<std::string>()->required()
    , "host register server is running on"
    )
    ( option::register_port
    , boost::program_options::value
        <fhg::util::boost::program_options::positive_integral<unsigned short>>()
        ->required()
    , "port register server is listening on"
    )
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
      .options (options_description)
      .run()
    , vm
    );

  boost::program_options::notify (vm);

  boost::optional<unsigned short> const port
    ( vm.count (option::port)
    ? boost::make_optional<unsigned short>
        ( vm.at (option::port)
        . as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
        )
    : boost::none
    );

  std::string const register_host
    (vm.at (option::register_host).as<std::string>());
  unsigned short const register_port
    ( vm.at (option::register_port)
      .as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
    );

  boost::asio::io_service io_service;

  fhg::rpc::service_dispatcher service_dispatcher
    {fhg::rpc::exception::serialization_functions()};

  fhg::rpc::service_handler<decltype (fhg::rif::execute_and_get_startup_messages)>
    execute_and_get_startup_messages_service
      ( service_dispatcher
      , "execute_and_get_startup_messages"
      , &fhg::rif::execute_and_get_startup_messages
      );

  fhg::rpc::service_handler<void (std::vector<pid_t>)> kill_service
    ( service_dispatcher
    , "kill"
    , [] (std::vector<pid_t> const& pids)
      {
        for (pid_t pid : pids)
        {
          fhg::syscall::kill (pid, SIGTERM);
        }
      }
    );

  std::vector<std::unique_ptr<fhg::network::connection_type>> connections;

  fhg::network::continous_acceptor<boost::asio::ip::tcp> acceptor
    ( boost::asio::ip::tcp::endpoint
        (boost::asio::ip::address(), port.get_value_or (0))
    , io_service
    , [] (fhg::network::buffer_type buf) { return buf; }
    , [] (fhg::network::buffer_type buf) { return buf; }
    , [&service_dispatcher]
        (fhg::network::connection_type* connection, fhg::network::buffer_type message)
      {
        service_dispatcher.dispatch (connection, message);
      }
    , [&connections] (fhg::network::connection_type* connection)
      {
        connections.erase
          ( std::find_if
              ( connections.begin()
              , connections.end()
              , [&connection]
                  (std::unique_ptr<fhg::network::connection_type> const& other)
                {
                  return other.get() == connection;
                }
              )
          );
      }
    , [&connections] (std::unique_ptr<fhg::network::connection_type> connection)
      {
        connections.emplace_back (std::move (connection));
      }
    );

  io_service.notify_fork (boost::asio::io_service::fork_prepare);
  if (pid_t child = fhg::syscall::fork())
  {
    io_service.notify_fork (boost::asio::io_service::fork_parent);

    boost::asio::io_service io_service;
    boost::asio::io_service::work const io_service_work (io_service);
    boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable> const
      io_service_thread ([&io_service] { io_service.run(); });

    fhg::rpc::remote_endpoint endpoint
      ( io_service
      , register_host, register_port
      , fhg::rpc::exception::serialization_functions()
      );

    struct stop_io_service_on_scope_exit
    {
      ~stop_io_service_on_scope_exit()
      {
        _io_service.stop();
      }
      boost::asio::io_service& _io_service;
    } stop_io_service_on_scope_exit {io_service};

    boost::asio::ip::tcp::endpoint const local_endpoint
      (acceptor.local_endpoint());

    fhg::rpc::sync_remote_function<void (fhg::rif::entry_point)>
      (endpoint, "register")
      ( fhg::rif::entry_point
          ( fhg::util::connectable_to_address_string (local_endpoint.address())
          , local_endpoint.port()
          , child
          )
      );

    return 0;
  }

  fhg::syscall::close (0);
  fhg::syscall::close (1);
  fhg::syscall::close (2);

  io_service.notify_fork (boost::asio::io_service::fork_child);

  const boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
    io_service_thread ([&io_service]() { io_service.run(); });

  return 0;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EX: ");

  return 1;
}
