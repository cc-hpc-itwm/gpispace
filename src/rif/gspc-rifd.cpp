#include <drts/certificates.hpp>

#include <rif/entry_point.hpp>

#include <vmem/netdev_id.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/syscall.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/join.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/temporary_file.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <logging/protocol.hpp>

#include <rif/execute_and_get_startup_messages.hpp>
#include <rif/protocol.hpp>
#include <rif/strategy/meta.hpp>

#include <rpc/future.hpp>
#include <rpc/remote_function.hpp>
#include <rpc/remote_socket_endpoint.hpp>
#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <fstream>
#include <stdexcept>

namespace
{
  namespace option
  {
    constexpr const char* const port {"port"};
    constexpr const char* const register_host {"register-host"};
    constexpr const char* const register_port {"register-port"};
    constexpr const char* const register_key {"register-key"};
  }

  void check_pid_is_still_running (pid_t pid)
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
  }

  void wait_for_pid_returned_with_exit_status_zero (pid_t pid)
  {
    int status;

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
    ( option::register_key
    , boost::program_options::value
        <fhg::util::boost::program_options::nonempty_string>()
        ->required()
    , "key to register with"
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
      ( static_cast<unsigned short>
        ( vm.at (option::port)
        . as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
        )
      )
    : boost::none
    );

  if (port)
  {
    throw std::logic_error ("NYI: gspc-rifd ignores given ports.");
  }

  std::string const register_host
    (vm.at (option::register_host).as<std::string>());
  unsigned short const register_port
    ( vm.at (option::register_port)
      .as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
    );
  std::string const register_key
    ( vm.at (option::register_key)
    . as<fhg::util::boost::program_options::nonempty_string>()
    );

  fhg::rpc::service_dispatcher service_dispatcher;

  fhg::rpc::service_handler<fhg::rif::protocol::execute_and_get_startup_messages>
    execute_and_get_startup_messages_service
      ( service_dispatcher
      , &fhg::rif::execute_and_get_startup_messages
      , fhg::rpc::not_yielding
      );

  fhg::rpc::service_handler<fhg::rif::protocol::execute_and_get_startup_messages_and_wait>
    execute_and_get_startup_messages_and_wait_service
      ( service_dispatcher
      , [] ( boost::filesystem::path command
           , std::vector<std::string> arguments
           , std::unordered_map<std::string, std::string> environment
           )
        {
          std::pair<pid_t, std::vector<std::string>> const
            pid_and_startup_messages
              ( fhg::rif::execute_and_get_startup_messages
                  (command, arguments, environment)
              );

          wait_for_pid_returned_with_exit_status_zero
            (pid_and_startup_messages.first);

          return pid_and_startup_messages.second;
        }
      , fhg::rpc::not_yielding
      );

  fhg::rpc::service_handler<fhg::rif::protocol::kill> kill_service
    ( service_dispatcher
    , [] (std::vector<pid_t> const& pids)
        -> std::unordered_map<pid_t, std::exception_ptr>
      {
        std::unordered_map<pid_t, std::exception_ptr> failures;
        for (pid_t pid : pids)
        {
          try
          {
            check_pid_is_still_running (pid);

            fhg::util::syscall::kill (pid, SIGTERM);

            wait_for_pid_returned_with_exit_status_zero (pid);
          }
          catch (...)
          {
            failures.emplace (pid, std::current_exception());
          }
        }
        return failures;
      }
    , fhg::rpc::not_yielding
    );

  fhg::rpc::service_handler<fhg::rif::protocol::start_vmem>
    start_vmem_service
      ( service_dispatcher
      , [] ( boost::filesystem::path command
           , boost::filesystem::path socket
           , unsigned short gaspi_port
           , std::chrono::seconds proc_init_timeout
           , std::vector<std::string> nodes
           , std::string gaspi_master
           , std::size_t rank
           , fhg::vmem::netdev_id netdev_id
           ) -> pid_t
        {
          std::vector<std::string> arguments
            { "--socket", socket.string()
            , "--port", std::to_string (gaspi_port)
            , "--gpi-timeout", std::to_string (proc_init_timeout.count())
            , "--netdev", to_string (netdev_id)
            };

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
                , { {"GASPI_MASTER", gaspi_master}
                  , {"GASPI_SOCKET", "0"}
                  , {"GASPI_MFILE", nodefile.string()}
                  , {"GASPI_RANK", std::to_string (rank)}
                  , {"GASPI_NRANKS", std::to_string (nodes.size())}
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
      , fhg::rpc::not_yielding
      );

  fhg::rpc::service_handler<fhg::rif::protocol::start_agent>
    start_agent_service
      ( service_dispatcher
      , [] ( std::string const& name
           , boost::optional<fhg::rif::protocol::hostinfo_t> const& parent
           , boost::optional<unsigned short> const& agent_port
           , boost::optional<boost::filesystem::path> const& gpi_socket
           , gspc::Certificates const& certificates
           , boost::filesystem::path const& command
           )
        {
          std::vector<std::string> arguments
            { "-u", "*:" + std::to_string (agent_port.get_value_or (0))
            , "-n", name
            };
          if (parent)
          {
            arguments.emplace_back ("--masters");
            arguments.emplace_back
              (parent->first + "%" + std::to_string (parent->second));
          }
          if (gpi_socket)
          {
            arguments.emplace_back ("--vmem-socket");
            arguments.emplace_back (gpi_socket->string());
          }
          if (certificates)
          {
            arguments.emplace_back ("--ssl-certificates");
            arguments.emplace_back (certificates->string());
          }

          auto const pid_and_startup_messages
            ( fhg::rif::execute_and_get_startup_messages
                ( command
                , arguments
                , std::unordered_map<std::string, std::string>()
                )
            );
          auto const& messages (pid_and_startup_messages.second);

          if (messages.size() != 3)
          {
            throw std::logic_error ( "could not start agent " + name
                                   + ": expected 3 lines of startup messages"
                                   );
          }

          fhg::rif::protocol::start_scheduler_result result;
          result.pid = pid_and_startup_messages.first;
          result.hostinfo
            = {messages[0], boost::lexical_cast<unsigned short> (messages[1])};
          result.logger_registration_endpoint = messages[2];
          return result;
        }
      , fhg::rpc::not_yielding
      );

  fhg::rpc::service_handler<fhg::rif::protocol::start_worker>
    start_worker_service
      ( service_dispatcher
      , [] ( std::string name
           , boost::filesystem::path command
           , std::vector<std::string> arguments
           , std::unordered_map<std::string, std::string> environment
           )
        {
          arguments.emplace_back ("-n");
          arguments.emplace_back (name);

          arguments.emplace_back ("--backlog-length");
          arguments.emplace_back ("1");

          auto const pid_and_startup_messages
            ( fhg::rif::execute_and_get_startup_messages
                (command, arguments, environment)
            );
          auto const& messages (pid_and_startup_messages.second);

          if (messages.size() != 1)
          {
            throw std::logic_error ( "could not start worker " + name
                                   + ": expected 1 line of startup messages"
                                   );
          }

          fhg::rif::protocol::start_worker_result result;
          result.pid = pid_and_startup_messages.first;
          result.logger_registration_endpoint = messages[0];
          return result;
        }
      , fhg::rpc::not_yielding
      );

  fhg::util::scoped_boost_asio_io_service_with_threads_and_deferred_startup
    io_service (1);

  std::unordered_map<pid_t, fhg::rpc::remote_socket_endpoint>
    add_emitters_endpoints;

  fhg::rpc::service_handler<fhg::rif::protocol::start_logging_demultiplexer>
    start_logging_demultiplexer_service
      ( service_dispatcher
      , [&add_emitters_endpoints, &io_service]
          (boost::asio::yield_context yield, boost::filesystem::path exe)
        {
          auto const pid_and_startup_messages
            ( fhg::rif::execute_and_get_startup_messages
                (exe, {}, std::unordered_map<std::string, std::string>())
            );
          auto const& messages (pid_and_startup_messages.second);

          if (messages.size() != 2)
          {
            throw std::logic_error ( "could not start logging-demultiplexer "
                                     ": expected 2 lines of startup messages"
                                   );
          }

          fhg::rif::protocol::start_logging_demultiplexer_result result;
          result.pid = pid_and_startup_messages.first;
          result.sink_endpoint = messages[0];

          add_emitters_endpoints.emplace
            ( std::piecewise_construct
            , std::forward_as_tuple (result.pid)
            , std::forward_as_tuple
                ( io_service
                , yield
                , fhg::logging::socket_endpoint (messages[1]).socket
                )
            );

          return result;
        }
      , fhg::rpc::yielding
      );

  fhg::rpc::service_handler<fhg::rif::protocol::add_emitter_to_logging_demultiplexer>
    add_emitter_to_logging_demultiplexer_service
      ( service_dispatcher
      , [&add_emitters_endpoints]
          ( boost::asio::yield_context yield
          , pid_t pid
          , std::vector<fhg::logging::endpoint> emitters
          )
        {
          auto const it (add_emitters_endpoints.find (pid));
          if (it == add_emitters_endpoints.end())
          {
            throw std::invalid_argument
              ("unknown log demultiplexer: " + std::to_string (pid));
          }

          fhg::rpc::sync_remote_function
            < fhg::logging::protocol::receiver::add_emitters
            , fhg::rpc::future
            > {it->second} (yield, std::move (emitters));
        }
      , fhg::rpc::yielding
      );

  fhg::rpc::service_tcp_provider_with_deferred_start server
    (io_service, service_dispatcher);

  if (pid_t child = fhg::util::syscall::fork())
  {
    io_service.post_fork_parent();

    fhg::util::scoped_boost_asio_io_service_with_threads io_service_parent (1);
    fhg::rpc::remote_tcp_endpoint endpoint
      (io_service_parent, register_host, register_port);

    boost::asio::ip::tcp::endpoint const local_endpoint
      (server.local_endpoint());

    fhg::rpc::sync_remote_function<fhg::rif::strategy::bootstrap_callback>
      {endpoint}
      ( register_key
      , fhg::util::hostname()
      , fhg::rif::entry_point
          ( fhg::util::connectable_to_address_string (local_endpoint.address())
          , local_endpoint.port()
          , child
          )
      );

    return 0;
  }

  fhg::util::syscall::setsid();

  fhg::util::syscall::close (0);
  fhg::util::syscall::close (1);
  fhg::util::syscall::close (2);

  server.start();
  io_service.post_fork_child();
  io_service.start_in_threads_and_current_thread();

  return 0;
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';

  return 1;
}
