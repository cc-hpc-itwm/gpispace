#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <fhglog/Configuration.hpp>
#include <fhglog/LogMacros.hpp>

#include <network/connectable_to_address_string.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/print_exception.hpp>

#include <rif/startup_messages_pipe.hpp>

#include <boost/program_options.hpp>

#include <sdpa/daemon/agent/Agent.hpp>
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
  }
}

int main (int argc, char **argv)
try
{
  namespace validators = fhg::util::boost::program_options;

  std::string agentName;
  std::string agentUrl;
  std::vector<std::string> arrMasterNames;
  std::string appGuiUrl;
  boost::optional<bfs::path> vmem_socket;

  boost::asio::io_service remote_log_io_service;
  fhg::log::Logger logger;
  fhg::log::configure (remote_log_io_service, logger);

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "Display this message")
    ("name,n", po::value<std::string>(&agentName)->default_value("agent"), "Agent's logical name")
    ("url,u",  po::value<std::string>(&agentUrl)->default_value("localhost"), "Agent's url")
    ("master,m", po::value<std::vector<std::string>>(&arrMasterNames)->multitoken(), "Agent's master list, of format 'name%host%port'")
    ("app_gui_url,a", po::value<std::string>(&appGuiUrl)->default_value("127.0.0.1:9000"), "application GUI's url")
    ( option_name::vmem_socket
    , boost::program_options::value<validators::nonempty_string>()
    , "socket file to communicate with the virtual memory manager"
    )
    ;

  desc.add (fhg::rif::startup_messages_pipe::program_options());

  po::variables_map vm;
  po::store( po::command_line_parser( argc, argv ).options(desc).run(), vm );

  if (vm.count ("help"))
  {
    LLOG (ERROR, logger, "usage: agent [options] ....");
    LLOG (ERROR, logger, desc);
    return 0;
  }

  po::notify (vm);

  if (vm.count (option_name::vmem_socket))
  {
    vmem_socket = bfs::path (vm.at (option_name::vmem_socket).as<validators::nonempty_string>());
  }

  std::vector<std::tuple<std::string, fhg::com::host_t, fhg::com::port_t>> masters;
  for (std::string const& name_host_port : arrMasterNames)
  {
    boost::tokenizer<boost::char_separator<char>> const tok
      (name_host_port, boost::char_separator<char> ("%"));

    std::vector<std::string> const parts (tok.begin(), tok.end());

    if (parts.size() != 3)
    {
      throw std::runtime_error
        ("invalid master information: has to be of format 'name%host%port'");
    }

    masters.emplace_back
      (parts[0], fhg::com::host_t (parts[1]), fhg::com::port_t (parts[2]));
  }

  boost::asio::io_service gui_io_service;
  const sdpa::daemon::Agent agent
    ( agentName
    , agentUrl
    , fhg::util::cxx14::make_unique<boost::asio::io_service>()
    , vmem_socket
    , masters
    , std::pair<std::string, boost::asio::io_service&> (appGuiUrl, gui_io_service)
    , logger
    );

  fhg::util::thread::event<> stop_requested;
  const std::function<void()> request_stop
    (std::bind (&fhg::util::thread::event<>::notify, &stop_requested));

  fhg::util::signal_handler_manager signal_handlers;
  fhg::util::scoped_log_backtrace_and_exit_for_critical_errors const
    crit_error_handler (signal_handlers, logger);

  fhg::util::scoped_signal_handler const SIGTERM_handler
    (signal_handlers, SIGTERM, std::bind (request_stop));
  fhg::util::scoped_signal_handler const SIGINT_handler
    (signal_handlers, SIGINT, std::bind (request_stop));

  {
    fhg::rif::startup_messages_pipe startup_messages_pipe (vm);
    startup_messages_pipe << fhg::network::connectable_to_address_string
                               (agent.peer_local_endpoint().address());
    startup_messages_pipe << agent.peer_local_endpoint().port();
  }

  stop_requested.wait();
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';
  return 1;
}
