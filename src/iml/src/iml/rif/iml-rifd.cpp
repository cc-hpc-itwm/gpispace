// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/rif/EntryPoint.hpp>

#include <iml/detail/Installation.hpp>
#include <iml/gaspi/NetdevID.hpp>

#include <util-generic/boost/program_options/validators/nonempty_string.hpp>
#include <util-generic/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/exit_status.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/join.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/temporary_file.hpp>

#include <util-generic/serialization/boost/filesystem/path.hpp>

#include <iml/rif/execute_and_get_startup_messages.hpp>
#include <iml/rif/protocol.hpp>
#include <iml/rif/strategy/meta.hpp>

#include <util-rpc/future.hpp>
#include <util-rpc/remote_function.hpp>
#include <util-rpc/remote_socket_endpoint.hpp>
#include <util-rpc/remote_tcp_endpoint.hpp>
#include <util-rpc/service_dispatcher.hpp>
#include <util-rpc/service_handler.hpp>
#include <util-rpc/service_tcp_provider.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/program_options.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/system/system_error.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <FMT/boost/filesystem/path.hpp>
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

    if (fhg::util::syscall::waitpid (pid, &status, WNOHANG) == pid)
    {
      if (fhg::util::wifexited (status))
      {
        throw std::runtime_error
          ("already returned " + std::to_string (fhg::util::wexitstatus (status)));
      }
      else if (fhg::util::wifsignaled (status))
      {
        throw std::runtime_error
          ("already signaled " + std::to_string (fhg::util::wtermsig (status)));
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
    if (fhg::util::wifexited (status) && fhg::util::wexitstatus (status))
    {
      throw std::runtime_error
        ("returned " + std::to_string (fhg::util::wexitstatus (status)));
    }
    else if (fhg::util::wifsignaled (status))
    {
      throw std::runtime_error
        ("signaled " + std::to_string (fhg::util::wtermsig (status)));
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
        <fhg::util::boost::program_options::positive_integral<unsigned short>>()
    , "port to listen on"
    )
    ( option::register_host
    , ::boost::program_options::value<std::string>()->required()
    , "host register server is running on"
    )
    ( option::register_port
    , ::boost::program_options::value
        <fhg::util::boost::program_options::positive_integral<unsigned short>>()
        ->required()
    , "port register server is listening on"
    )
    ( option::register_key
    , ::boost::program_options::value
        <fhg::util::boost::program_options::nonempty_string>()
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

  ::boost::optional<unsigned short> const port
    ( vm.count (option::port)
    ? ::boost::make_optional<unsigned short>
      ( static_cast<unsigned short>
        ( vm.at (option::port)
        . as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
        )
      )
    : ::boost::none
    );

  if (port)
  {
    throw std::logic_error ("NYI: iml-rifd ignores given ports.");
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

  fhg::rpc::service_handler<fhg::iml::rif::protocol::kill> kill_service
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

  iml::detail::Installation const installation;

  fhg::rpc::service_handler<fhg::iml::rif::protocol::start_vmem>
    start_vmem_service
      ( service_dispatcher
      , [&] ( ::boost::filesystem::path socket
            , unsigned short gaspi_port
            , std::chrono::seconds proc_init_timeout
            , std::vector<std::string> nodes
            , std::size_t rank
            , iml::gaspi::NetdevID netdev_id
            ) -> pid_t
        {
          std::vector<std::string> arguments
            { "--socket", socket.string()
            , "--port", std::to_string (gaspi_port)
            , "--gpi-timeout", std::to_string (proc_init_timeout.count())
            , "--netdev", netdev_id.to_string()
            };

          //! \todo allow to specify folder to put temporary file in
          ::boost::filesystem::path const nodefile
            ( ::boost::filesystem::unique_path
                ("IML-VMEM-NODEFILE-%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%")
            );
          fhg::util::temporary_file nodefile_temporary (nodefile);
          {
            std::ofstream nodefile_stream (nodefile.string());

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
                  ( "Could not write to nodefile {}: {}"
                  , nodefile
                  , strerror (errno)
                  )
                };
            }
          }

          std::pair<pid_t, std::vector<std::string>> const
            startup_messages
            ( fhg::iml::rif::execute_and_get_startup_messages
                ( installation.server_binary
                , arguments
                , { {"GASPI_SOCKET", "0"}
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

    ::boost::asio::ip::tcp::endpoint const local_endpoint
      (server.local_endpoint());

    fhg::rpc::sync_remote_function<fhg::iml::rif::strategy::bootstrap_callback>
      {endpoint}
      ( register_key
      , fhg::util::hostname()
      , iml::rif::EntryPoint
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
