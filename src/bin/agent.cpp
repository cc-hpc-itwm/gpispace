// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/getenv.hpp>
#include <util-generic/print_exception.hpp>

#include <rif/started_process_promise.hpp>

#include <boost/program_options.hpp>

#include <sdpa/daemon/Agent.hpp>
#include <we/layer.hpp>
#include <boost/filesystem/path.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/tokenizer.hpp>

#include <functional>

namespace bfs = boost::filesystem;
namespace po = boost::program_options;

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
    std::vector<std::string> arrMasterNames;
    boost::optional<bfs::path> vmem_socket;
    fhg::com::Certificates ssl_certificates;

    po::options_description desc("Allowed options");
    desc.add_options()
      ("name,n", po::value<std::string>(&agentName)->default_value("agent"), "Agent's logical name")
      ("url,u",  po::value<std::string>(&agentUrl)->default_value("localhost"), "Agent's url")
      ("masters", po::value<std::vector<std::string>>(&arrMasterNames)->multitoken(), "Agent's master list, of format 'host%port'")
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
    po::store( po::command_line_parser( argc, argv ).options(desc).run(), vm );

    po::notify (vm);

    if (vm.count (option_name::vmem_socket))
    {
      vmem_socket = bfs::path (vm.at (option_name::vmem_socket).as<validators::nonempty_string>());
    }

    if (vm.count (option_name::ssl_certificates))
    {
      ssl_certificates = vm.at (option_name::ssl_certificates).as<bfs::path>();
    }

    sdpa::master_info_t masters;
    for (auto const& host_port : arrMasterNames)
    {
      boost::tokenizer<boost::char_separator<char>> const tok
        (host_port, boost::char_separator<char> ("%"));

      std::vector<std::string> const parts (tok.begin(), tok.end());

      if (parts.size() != 2)
      {
        throw std::runtime_error
          ("invalid master information: has to be of format 'host%port'");
      }

      masters.emplace_front (parts[0], parts[1]);
    }

    sdpa::daemon::Agent agent
      ( agentName
      , agentUrl
      , fhg::util::cxx14::make_unique<boost::asio::io_service>()
      , vmem_socket
      , std::move (masters)
      , true
      , ssl_certificates
      );

    fhg::util::thread::event<> stop_requested;
    const std::function<void()> request_stop
      (std::bind (&fhg::util::thread::event<>::notify, &stop_requested));

    fhg::util::signal_handler_manager signal_handlers;
    fhg::util::scoped_log_backtrace_and_exit_for_critical_errors const
      crit_error_handler (signal_handlers, agent.log_emitter());

    fhg::util::scoped_signal_handler const SIGTERM_handler
      (signal_handlers, SIGTERM, std::bind (request_stop));
    fhg::util::scoped_signal_handler const SIGINT_handler
      (signal_handlers, SIGINT, std::bind (request_stop));

    promise.set_result ( fhg::util::connectable_to_address_string
                           (agent.peer_local_endpoint().address())
                       , std::to_string
                           (agent.peer_local_endpoint().port())
                       , agent.logger_registration_endpoint().to_string()
                       );

    stop_requested.wait();

    return EXIT_SUCCESS;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());

    return EXIT_FAILURE;
  }
}
