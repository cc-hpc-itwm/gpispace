// Copyright (C) 2015-2016,2018-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/drts/certificates.hpp>

#include <gspc/rif/entry_point.hpp>

#include <gspc/util/boost/program_options/validators/nonempty_string.hpp>
#include <gspc/util/boost/program_options/validators/positive_integral.hpp>
#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/exit_status.hpp>
#include <gspc/util/hostname.hpp>
#include <gspc/util/join.hpp>
#include <gspc/util/print_exception.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/util/syscall.hpp>
#include <gspc/util/temporary_file.hpp>

#include <gspc/logging/protocol.hpp>

#include <gspc/rif/execute_and_get_startup_messages.hpp>
#include <gspc/rif/protocol.hpp>
#include <gspc/rif/strategy/meta.hpp>

#include <gspc/rpc/future.hpp>
#include <gspc/rpc/remote_function.hpp>
#include <gspc/rpc/remote_socket_endpoint.hpp>
#include <gspc/rpc/remote_tcp_endpoint.hpp>
#include <gspc/rpc/service_dispatcher.hpp>
#include <gspc/rpc/service_handler.hpp>
#include <gspc/rpc/service_tcp_provider.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <chrono>
#include <optional>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>
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

    if (gspc::util::syscall::waitpid (pid, &status, WNOHANG) == pid)
    {
      if (gspc::util::wifexited (status))
      {
        throw std::runtime_error
          ("already returned " + std::to_string (gspc::util::wexitstatus (status)));
      }
      else if (gspc::util::wifsignaled (status))
      {
        throw std::runtime_error
          ("already signaled " + std::to_string (gspc::util::wtermsig (status)));
      }
    }
  }

  void wait_for_pid_returned_with_exit_status_zero (pid_t pid)
  {
    int status;

    if (gspc::util::syscall::waitpid (pid, &status, 0) != pid)
    {
      throw std::logic_error ("waitpid returned for wrong child");
    }
    if (gspc::util::wifexited (status) && gspc::util::wexitstatus (status))
    {
      throw std::runtime_error
        ("returned " + std::to_string (gspc::util::wexitstatus (status)));
    }
    else if (gspc::util::wifsignaled (status))
    {
      throw std::runtime_error
        ("signaled " + std::to_string (gspc::util::wtermsig (status)));
    }
  }
}

int main (int argc, char** argv)
try
{
  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ( option::port
    , ::boost::program_options::value
        <gspc::util::boost::program_options::positive_integral<unsigned short>>()
    , "port to listen on"
    )
    ( option::register_host
    , ::boost::program_options::value<std::string>()->required()
    , "host register server is running on"
    )
    ( option::register_port
    , ::boost::program_options::value
        <gspc::util::boost::program_options::positive_integral<unsigned short>>()
        ->required()
    , "port register server is listening on"
    )
    ( option::register_key
    , ::boost::program_options::value
        <gspc::util::boost::program_options::nonempty_string>()
        ->required()
    , "key to register with"
    )
    ;

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser (argc, argv)
      .options (options_description)
      .run()
    , vm
    );

  ::boost::program_options::notify (vm);

  std::optional<unsigned short> const port
    ( vm.count (option::port)
    ? std::make_optional<unsigned short>
      ( static_cast<unsigned short>
        ( vm.at (option::port)
        . as<gspc::util::boost::program_options::positive_integral<unsigned short>>()
        )
      )
    : std::nullopt
    );

  if (port)
  {
    throw std::logic_error ("NYI: gspc-rifd ignores given ports.");
  }

  std::string const register_host
    (vm.at (option::register_host).as<std::string>());
  unsigned short const register_port
    ( vm.at (option::register_port)
      .as<gspc::util::boost::program_options::positive_integral<unsigned short>>()
    );
  std::string const register_key
    ( vm.at (option::register_key)
    . as<gspc::util::boost::program_options::nonempty_string>()
    );

  gspc::rpc::service_dispatcher service_dispatcher;

  gspc::rpc::service_handler<gspc::rif::protocol::execute_and_get_startup_messages>
    execute_and_get_startup_messages_service
      ( service_dispatcher
      , &gspc::rif::execute_and_get_startup_messages
      , gspc::rpc::not_yielding
      );

  gspc::rpc::service_handler<gspc::rif::protocol::execute_and_get_startup_messages_and_wait>
    execute_and_get_startup_messages_and_wait_service
      ( service_dispatcher
      , [] ( std::filesystem::path command
           , std::vector<std::string> arguments
           , std::unordered_map<std::string, std::string> environment
           )
        {
          auto const startup_result
            ( gspc::rif::execute_and_get_startup_messages
                (command, arguments, environment)
            );

          wait_for_pid_returned_with_exit_status_zero (startup_result.pid);

          return startup_result.messages;
        }
      , gspc::rpc::not_yielding
      );

  gspc::rpc::service_handler<gspc::rif::protocol::kill> kill_service
    ( service_dispatcher
    , [] (std::vector<pid_t> const& pids)
        -> std::unordered_map<pid_t, std::exception_ptr>
      {
        std::unordered_map<pid_t, std::exception_ptr> failures;
        for (auto pid : pids)
        {
          try
          {
            check_pid_is_still_running (pid);

            gspc::util::syscall::kill (pid, SIGTERM);

            wait_for_pid_returned_with_exit_status_zero (pid);
          }
          catch (...)
          {
            failures.emplace (pid, std::current_exception());
          }
        }
        return failures;
      }
    , gspc::rpc::not_yielding
    );

  gspc::rpc::service_handler<gspc::rif::protocol::start_agent>
    start_agent_service
      ( service_dispatcher
       , [] ( std::string const& name
         , std::optional<unsigned short> const& agent_port
         , std::optional<std::filesystem::path> const& gpi_socket
         , gspc::Certificates const& certificates
         , std::filesystem::path const& command
         )
        {
          std::vector<std::string> arguments
            { "-u", "*:" + std::to_string (agent_port.value_or (0))
            , "-n", name
            };
          if (gpi_socket)
          {
            arguments.emplace_back ("--vmem-socket");
            arguments.emplace_back (gpi_socket->string());
          }
          if (certificates.path.has_value())
          {
            arguments.emplace_back ("--ssl-certificates");
            arguments.emplace_back (certificates.path->string());
          }

          auto const startup_result
            ( gspc::rif::execute_and_get_startup_messages
                ( command
                , arguments
                , std::unordered_map<std::string, std::string>()
                )
            );
          auto const& messages (startup_result.messages);

          if (messages.size() != 3)
          {
            throw std::logic_error ( "could not start agent " + name
                                   + ": expected 3 lines of startup messages"
                                   );
          }

          gspc::rif::protocol::start_scheduler_result result;
          result.pid = startup_result.pid;
          result.hostinfo
            = {messages[0], ::boost::lexical_cast<unsigned short> (messages[1])};
          result.logger_registration_endpoint = messages[2];
          return result;
        }
      , gspc::rpc::not_yielding
      );

  gspc::rpc::service_handler<gspc::rif::protocol::start_worker>
    start_worker_service
      ( service_dispatcher
      , [] ( std::string name
           , std::filesystem::path command
           , std::vector<std::string> arguments
           , std::unordered_map<std::string, std::string> environment
           )
        {
          arguments.emplace_back ("-n");
          arguments.emplace_back (name);

          auto const startup_result
            ( gspc::rif::execute_and_get_startup_messages
                (command, arguments, environment)
            );
          auto const& messages (startup_result.messages);

          if (messages.size() != 1)
          {
            throw std::logic_error ( "could not start worker " + name
                                   + ": expected 1 line of startup messages"
                                   );
          }

          gspc::rif::protocol::start_worker_result result;
          result.pid = startup_result.pid;
          result.logger_registration_endpoint = messages[0];
          return result;
        }
      , gspc::rpc::not_yielding
      );

  gspc::rpc::service_handler<gspc::rif::protocol::start_vmem>
    start_vmem_service
      ( service_dispatcher
      , [] ( std::filesystem::path command
           , std::filesystem::path socket
           , unsigned short gaspi_port
           , std::chrono::seconds proc_init_timeout
           , std::vector<std::string> nodes
           , std::size_t rank
           , std::string netdev_id
           ) -> pid_t
        {
          std::vector<std::string> arguments
            { "--socket", socket.string()
            , "--port", std::to_string (gaspi_port)
            , "--gpi-timeout"
            , std::to_string (proc_init_timeout.count())
            , "--netdev", netdev_id
            };

          std::filesystem::path const nodefile
            { std::filesystem::temp_directory_path()
            / fmt::format
                ( "gspc-rifd-vmem-nodefile-{}-{}"
                , gspc::util::syscall::getpid()
                , std::chrono::high_resolution_clock::now()
                  .time_since_epoch().count()
                )
            };
          gspc::util::temporary_file const
            nodefile_temporary (nodefile);
          {
            std::ofstream nodefile_stream (nodefile);

            if (!nodefile_stream)
            {
              throw std::runtime_error
                { fmt::format
                    ( "Could not create nodefile {}: {}"
                    , nodefile
                    , strerror (errno)
                    )
                };
            }

            for (std::string const& node : nodes)
            {
              nodefile_stream << node << "\n";
            }

            if (!nodefile_stream)
            {
              throw std::runtime_error
                { fmt::format
                    ( "Could not write to"
                      " nodefile {}: {}"
                    , nodefile
                    , strerror (errno)
                    )
                };
            }
          }

          auto const startup_result
            ( gspc::rif::execute_and_get_startup_messages
                ( command
                , arguments
                , { {"GASPI_SOCKET", "0"}
                  , { "GASPI_MFILE"
                    , nodefile.string()
                    }
                  , { "GASPI_RANK"
                    , std::to_string (rank)
                    }
                  , { "GASPI_NRANKS"
                    , std::to_string (nodes.size())
                    }
                  , {"GASPI_SET_NUMA_SOCKET", "0"}
                  }
                )
            );

          if (!startup_result.messages.empty())
          {
            throw std::logic_error
              ("expected no startup messages");
          }

          return startup_result.pid;
        }
      , gspc::rpc::not_yielding
      );

  gspc::util::scoped_boost_asio_io_service_with_threads_and_deferred_startup
    io_service (1);

  std::unordered_map<pid_t, gspc::rpc::remote_socket_endpoint>
    add_emitters_endpoints;

  gspc::rpc::service_handler<gspc::rif::protocol::start_logging_demultiplexer>
    start_logging_demultiplexer_service
      ( service_dispatcher
      , [&add_emitters_endpoints, &io_service]
          (::boost::asio::yield_context yield, std::filesystem::path exe)
        {
          auto const startup_result
            ( gspc::rif::execute_and_get_startup_messages
                (exe, {}, std::unordered_map<std::string, std::string>())
            );
          auto const& messages (startup_result.messages);

          if (messages.size() != 2)
          {
            throw std::logic_error ( "could not start logging-demultiplexer "
                                     ": expected 2 lines of startup messages"
                                   );
          }

          gspc::rif::protocol::start_logging_demultiplexer_result result;
          result.pid = startup_result.pid;
          result.sink_endpoint = messages[0];

          add_emitters_endpoints.emplace
            ( std::piecewise_construct
            , std::forward_as_tuple (result.pid)
            , std::forward_as_tuple
                ( io_service
                , yield
                , gspc::logging::socket_endpoint (messages[1]).socket
                )
            );

          return result;
        }
      , gspc::rpc::yielding
      );

  gspc::rpc::service_handler<gspc::rif::protocol::add_emitter_to_logging_demultiplexer>
    add_emitter_to_logging_demultiplexer_service
      ( service_dispatcher
      , [&add_emitters_endpoints]
          ( ::boost::asio::yield_context yield
          , pid_t pid
          , std::vector<gspc::logging::endpoint> emitters
          )
        {
          auto const it (add_emitters_endpoints.find (pid));
          if (it == add_emitters_endpoints.end())
          {
            throw std::invalid_argument
              ("unknown log demultiplexer: " + std::to_string (pid));
          }

          gspc::rpc::sync_remote_function
            < gspc::logging::protocol::receiver::add_emitters
            , gspc::rpc::future
            > {it->second} (yield, std::move (emitters));
        }
      , gspc::rpc::yielding
      );

  gspc::rpc::service_tcp_provider_with_deferred_start server
    (io_service, service_dispatcher);

  if (pid_t child = gspc::util::syscall::fork())
  {
    io_service.post_fork_parent();

    gspc::util::scoped_boost_asio_io_service_with_threads io_service_parent (1);
    gspc::rpc::remote_tcp_endpoint endpoint
      (io_service_parent, register_host, register_port);

    ::boost::asio::ip::tcp::endpoint const local_endpoint
      (server.local_endpoint());

    gspc::rpc::sync_remote_function<gspc::rif::strategy::bootstrap_callback>
      {endpoint}
      ( register_key
      , gspc::util::hostname()
      , gspc::rif::entry_point
          ( gspc::util::connectable_to_address_string (local_endpoint.address())
          , local_endpoint.port()
          , child
          )
      );

    return 0;
  }

  gspc::util::syscall::setsid();

  gspc::util::syscall::close (0);
  gspc::util::syscall::close (1);
  gspc::util::syscall::close (2);

  server.start();
  io_service.post_fork_child();
  io_service.start_in_threads_and_current_thread();

  return 0;
}
catch (...)
{
  std::cerr << "EX: " << gspc::util::current_exception_printer() << '\n';

  return 1;
}
