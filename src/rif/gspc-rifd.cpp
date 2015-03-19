// bernd.loerwald@itwm.fraunhofer.de

#include <rif/entry_point.hpp>

#include <util-generic/syscall.hpp>
#include <network/connectable_to_address_string.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/join.hpp>
#include <util-generic/print_exception.hpp>
#include <fhg/util/temporary_file.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/serialization/std/unordered_map.hpp>

#include <fhglog/level_io.hpp>

#include <network/server.hpp>

#include <rif/execute_and_get_startup_messages.hpp>
#include <rif/protocol.hpp>

#include <rpc/server.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <fstream>

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
    {fhg::util::serialization::exception::serialization_functions()};

  fhg::rpc::service_handler<fhg::rif::protocol::execute_and_get_startup_messages>
    execute_and_get_startup_messages_service
      (service_dispatcher, &fhg::rif::execute_and_get_startup_messages);

  fhg::rpc::service_handler<fhg::rif::protocol::kill> kill_service
    ( service_dispatcher
    , [] (std::vector<pid_t> const& pids)
      {
        std::vector<std::string> failed_statuses;
        for (pid_t pid : pids)
        {
          try
          {
            int status;

            if (fhg::util::syscall::waitpid (pid, &status, WNOHANG) == pid)
            {
              if (WIFEXITED (status))
              {
                throw std::runtime_error
                  ("already returned " + std::to_string (WEXITSTATUS (status)));
              }
              else if (WIFSIGNALED (status))
              {
                throw std::runtime_error
                  ("already signaled " + std::to_string (WTERMSIG (status)));
              }
            }

            fhg::util::syscall::kill (pid, SIGTERM);

            if (fhg::util::syscall::waitpid (pid, &status, 0) != pid)
            {
              throw std::logic_error ("waitpid returned for wrong child");
            }
            if (WIFEXITED (status) && WEXITSTATUS (status))
            {
              throw std::runtime_error
                ("returned " + std::to_string (WEXITSTATUS (status)));
            }
            else if (WIFSIGNALED (status))
            {
              throw std::runtime_error
                ("signaled " + std::to_string (WTERMSIG (status)));
            }
          }
          catch (...)
          {
            std::ostringstream oss;
            fhg::util::print_current_exception
              (oss, std::to_string (pid) + ": ");
            failed_statuses.emplace_back (oss.str());
          }
        }
        if (!failed_statuses.empty())
        {
          throw std::runtime_error (fhg::util::join (failed_statuses, ", "));
        }
      }
    );

  fhg::rpc::service_handler<fhg::rif::protocol::start_vmem>
    start_vmem_service
      ( service_dispatcher
      , [] ( boost::filesystem::path command
           , fhg::log::Level log_level
           , std::size_t memory_in_bytes
           , boost::filesystem::path socket
           , unsigned short gaspi_port
           , std::chrono::seconds proc_init_timeout
           , std::string vmem_implementation
           , boost::optional<std::pair<std::string, unsigned short>> log_server
           , boost::optional<boost::filesystem::path> log_file
           , std::vector<std::string> nodes
           , std::string gaspi_master
           , bool is_master
           ) -> pid_t
        {
          std::vector<std::string> arguments
            { "--log-level", fhg::log::string (log_level)
            , "--gpi-mem", std::to_string (memory_in_bytes)
            , "--socket", socket.string()
            , "--port", std::to_string (gaspi_port)
            , "--gpi-api", vmem_implementation
            , "--gpi-timeout", std::to_string (proc_init_timeout.count())
            };
          if (log_server)
          {
            arguments.emplace_back ("--log-host");
            arguments.emplace_back (log_server->first);
            arguments.emplace_back ("--log-port");
            arguments.emplace_back (std::to_string (log_server->second));
          }
          if (log_file)
          {
            arguments.emplace_back ("--log-file");
            arguments.emplace_back (log_file->string());
          }

          //! \todo allow to specify folder to put temporary file in
          boost::filesystem::path const nodefile
            ( boost::filesystem::unique_path
                ("GPISPACE-VMEM-NODEFILE-%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
            );
          fhg::util::temporary_file nodefile_temporary (nodefile);
          {
            std::ofstream nodefile_stream (nodefile.string());

            if (!nodefile_stream)
            {
              throw std::runtime_error
                ( ( boost::format ("Could not create nodefile %1%: %2%")
                  % nodefile
                  % strerror (errno)
                  )
                . str()
                );
            }

            for (std::string const& node : nodes)
            {
              nodefile_stream << node << "\n";
            }

            if (!nodefile_stream)
            {
              throw std::runtime_error
                ( ( boost::format ("Could not write to nodefile %1%: %2%")
                  % nodefile
                  % strerror (errno)
                  )
                . str()
                );
            }
          }

          std::pair<pid_t, std::vector<std::string>> const
            startup_messages
            ( fhg::rif::execute_and_get_startup_messages
                ( command
                , arguments
                , { {"GASPI_MFILE", nodefile.string()}
                  , {"GASPI_MASTER", gaspi_master}
                  , {"GASPI_SOCKET", "0"}
                  , {"GASPI_TYPE", is_master ? "GASPI_MASTER" : "GASPI_WORKER"}
                  , {"GASPI_SET_NUMA_SOCKET", "0"}
                  }
                )
            );

          if (!startup_messages.second.empty())
          {
            throw std::logic_error ("expected no startup messages");
          }

          return startup_messages.first;
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
  if (pid_t child = fhg::util::syscall::fork())
  {
    io_service.notify_fork (boost::asio::io_service::fork_parent);

    boost::asio::io_service io_service;
    boost::asio::io_service::work const io_service_work (io_service);
    boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable> const
      io_service_thread ([&io_service] { io_service.run(); });

    fhg::rpc::remote_endpoint endpoint
      ( io_service
      , register_host, register_port
      , fhg::util::serialization::exception::serialization_functions()
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
          ( fhg::network::connectable_to_address_string (local_endpoint.address())
          , local_endpoint.port()
          , child
          )
      );

    return 0;
  }

  fhg::util::syscall::close (0);
  fhg::util::syscall::close (1);
  fhg::util::syscall::close (2);

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
