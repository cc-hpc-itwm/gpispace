// Copyright (C) 2011,2014-2016,2018-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/util/signal_handler_manager.hpp>
#include <gspc/rif/started_process_promise.hpp>
#include <gspc/scheduler/daemon/Agent.hpp>
#include <gspc/we/layer.hpp>

#include <gspc/util/boost/program_options/validators/existing_path.hpp>
#include <gspc/util/boost/program_options/validators/nonempty_string.hpp>
#include <gspc/util/connectable_to_address_string.hpp>
#include <gspc/util/print_exception.hpp>

#include <boost/program_options.hpp>

#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>

namespace
{
  namespace option_name
  {
    constexpr const char* vmem_socket {"vmem-socket"};
    constexpr const char* ssl_certificates {"ssl-certificates"};
  }
}

int main (int argc, char **argv)
{
  gspc::rif::started_process_promise promise (argc, argv);

  try
  {
    namespace validators = gspc::util::boost::program_options;

    std::string agentName;
    std::string agentUrl;
    std::optional<std::filesystem::path> vmem_socket;
    gspc::Certificates ssl_certificates;

    ::boost::program_options::options_description desc("Allowed options");
    desc.add_options()
      ("name,n", ::boost::program_options::value<std::string>(&agentName)->default_value("agent"), "Agent's logical name")
      ("url,u",  ::boost::program_options::value<std::string>(&agentUrl)->default_value("localhost"), "Agent's url")
      ( option_name::vmem_socket
      , ::boost::program_options::value<validators::nonempty_string>()
      , "socket file to communicate with the virtual memory manager"
      )
      ( option_name::ssl_certificates
      , ::boost::program_options::value<std::filesystem::path>()
      , "folder containing SSL certificates"
      )
      ;

    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (argc, argv)
          .options (desc)
          .run()
      , vm
      );

    ::boost::program_options::notify (vm);

    if (vm.count (option_name::vmem_socket))
    {
      vmem_socket = std::filesystem::path
        { std::string
          { vm.at (option_name::vmem_socket).as<validators::nonempty_string>()
          }
        };
    }

    if (vm.count (option_name::ssl_certificates))
    {
      ssl_certificates = vm.at
        ( option_name::ssl_certificates
        ).as<std::filesystem::path>()
        ;
    }

    gspc::scheduler::daemon::Agent agent
      ( agentName
      , agentUrl
      , std::make_unique<::boost::asio::io_service>()
      , vmem_socket
      , ssl_certificates
      );

    gspc::util::signal_handler_manager signal_handlers;
    gspc::util::scoped_log_backtrace_and_exit_for_critical_errors const
      crit_error_handler (signal_handlers, agent.log_emitter());

    gspc::util::Execution execution (signal_handlers);

    promise.set_result ( gspc::util::connectable_to_address_string
                           (agent.peer_local_endpoint().address())
                       , std::to_string
                           (agent.peer_local_endpoint().port())
                       , agent.logger_registration_endpoint().to_string()
                       );

    execution.wait();

    return EXIT_SUCCESS;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());

    return EXIT_FAILURE;
  }
}
