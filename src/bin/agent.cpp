// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/signal_handler_manager.hpp>
#include <rif/started_process_promise.hpp>
#include <sdpa/daemon/Agent.hpp>
#include <we/layer.hpp>

#include <util-generic/boost/program_options/validators/existing_path.hpp>
#include <util-generic/boost/program_options/validators/nonempty_string.hpp>
#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/print_exception.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>

#include <csignal>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>

namespace bfs = ::boost::filesystem;
namespace po = ::boost::program_options;

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
  fhg::rif::started_process_promise promise (argc, argv);

  try
  {
    namespace validators = fhg::util::boost::program_options;

    std::string agentName;
    std::string agentUrl;
    std::optional<bfs::path> vmem_socket;
    gspc::Certificates ssl_certificates;

    po::options_description desc("Allowed options");
    desc.add_options()
      ("name,n", po::value<std::string>(&agentName)->default_value("agent"), "Agent's logical name")
      ("url,u",  po::value<std::string>(&agentUrl)->default_value("localhost"), "Agent's url")
      ( option_name::vmem_socket
      , po::value<validators::nonempty_string>()
      , "socket file to communicate with the virtual memory manager"
      )
      ( option_name::ssl_certificates
      , po::value<bfs::path>()
      , "folder containing SSL certificates"
      )
      ;

    po::variables_map vm;
    po::store (po::command_line_parser (argc, argv).options (desc).run(), vm);

    po::notify (vm);

    if (vm.count (option_name::vmem_socket))
    {
      vmem_socket = bfs::path (vm.at (option_name::vmem_socket).as<validators::nonempty_string>());
    }

    if (vm.count (option_name::ssl_certificates))
    {
      ssl_certificates = vm.at (option_name::ssl_certificates).as<bfs::path>();
    }

    sdpa::daemon::Agent agent
      ( agentName
      , agentUrl
      , std::make_unique<::boost::asio::io_service>()
      , vmem_socket
      , ssl_certificates
      );

    fhg::util::signal_handler_manager signal_handlers;
    fhg::util::scoped_log_backtrace_and_exit_for_critical_errors const
      crit_error_handler (signal_handlers, agent.log_emitter());

    fhg::util::Execution execution (signal_handlers);

    promise.set_result ( fhg::util::connectable_to_address_string
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
