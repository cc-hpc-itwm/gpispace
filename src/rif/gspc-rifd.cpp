// bernd.loerwald@itwm.fraunhofer.de

#include <rif/entry_point.hpp>

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

#include <fhglog/level_io.hpp>

#include <rif/execute_and_get_startup_messages.hpp>
#include <rif/protocol.hpp>
#include <rif/strategy/meta.hpp>

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
  std::string const register_key
    ( vm.at (option::register_key)
    . as<fhg::util::boost::program_options::nonempty_string>()
    );

  fhg::rpc::service_dispatcher service_dispatcher;

  fhg::rpc::service_handler<fhg::rif::protocol::execute_and_get_startup_messages>
    execute_and_get_startup_messages_service
      (service_dispatcher, &fhg::rif::execute_and_get_startup_messages);

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
    );

  std::unordered_map<pid_t, std::string> pending_vmems;

  fhg::rpc::service_handler<fhg::rif::protocol::start_vmem_step_a>
    start_vmem_service_step_a
      ( service_dispatcher
      , [&] ( boost::filesystem::path command
            , boost::filesystem::path socket
            , unsigned short gaspi_port
            ,  std::chrono::seconds proc_init_timeout
            ) -> std::pair<pid_t, std::uint16_t>
        {
          std::pair<pid_t, std::vector<std::string>> const
            startup_messages
            ( fhg::rif::execute_and_get_startup_messages
                ( command
                , { "--socket", socket.string()
                  , "--port", std::to_string (gaspi_port)
                  , "--gpi-timeout", std::to_string (proc_init_timeout.count())
                  }
                , std::unordered_map<std::string, std::string> {}
                )
            );

          if (startup_messages.second.size() != 2)
          {
            throw std::logic_error ("expected two startup messages");
          }

          pending_vmems.emplace
            (startup_messages.first, startup_messages.second[0]);

          return { startup_messages.first
                 , std::stoi (startup_messages.second[1])
                 };
        }
      );

  fhg::rpc::service_handler<fhg::rif::protocol::start_vmem_step_b>
    start_vmem_service_step_b
      ( service_dispatcher
      , [&] (pid_t pid, std::vector<intertwine::vmem::node> nodes) -> void
        {
          fhg::util::scoped_boost_asio_io_service_with_threads setup_thread {1};
          fhg::rpc::remote_socket_endpoint endpoint
            { setup_thread
            , boost::asio::local::stream_protocol::endpoint
                (pending_vmems.at (pid))
            };
          fhg::rpc::sync_remote_function
            <fhg::rif::protocol::local::vmem_set_port_and_continue> {endpoint}
              (nodes);
        }
      );

  fhg::util::scoped_boost_asio_io_service_with_threads_and_deferred_startup
    io_service (1);
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
