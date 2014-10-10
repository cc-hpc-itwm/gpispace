#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <fhglog/LogMacros.hpp>

#include <boost/program_options.hpp>

#include <sdpa/daemon/agent/Agent.hpp>
#include <we/layer.hpp>
#include <boost/filesystem/path.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/pidfile_writer.hpp>
#include <fhg/util/read_bool.hpp>
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
{
  namespace validators = fhg::util::boost::program_options;

  std::string agentName;
  std::string agentUrl;
  std::vector<std::string> arrMasterNames;
  std::string arrMasterUrls;
  std::string appGuiUrl;
  std::string pidfile;
  boost::optional<bfs::path> vmem_socket;

  boost::asio::io_service remote_log_io_service;
  FHGLOG_SETUP (remote_log_io_service);

  po::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "Display this message")
    ("name,n", po::value<std::string>(&agentName)->default_value("agent"), "Agent's logical name")
    ("url,u",  po::value<std::string>(&agentUrl)->default_value("localhost"), "Agent's url")
    //("orch_name,m",  po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
    ("master,m", po::value<std::vector<std::string>>(&arrMasterNames)->multitoken(), "Agent's master list")
    ("app_gui_url,a", po::value<std::string>(&appGuiUrl)->default_value("127.0.0.1:9000"), "application GUI's url")
    ("kvs-host",  po::value<std::string>()->required(), "The kvs daemon's host")
    ("kvs-port",  po::value<std::string>()->required(), "The kvs daemon's port")
    ("pidfile", po::value<std::string>(&pidfile)->default_value(pidfile), "write pid to pidfile")
    ("daemonize", "daemonize after all checks were successful")
    ( option_name::vmem_socket
    , boost::program_options::value<validators::nonempty_string>()
    , "socket file to communicate with the virtual memory manager"
    )
    ;

  po::variables_map vm;
  po::store( po::command_line_parser( argc, argv ).options(desc).run(), vm );

  fhg::log::Logger::ptr_t logger (fhg::log::Logger::get (agentName));

  if (vm.count ("help"))
  {
    LLOG (ERROR, logger, "usage: agent [options] ....");
    LLOG (ERROR, logger, desc);
    return 0;
  }

  po::notify (vm);

  if (vm.count (option_name::vmem_socket))
  {
    vmem_socket = bfs::path (vm[option_name::vmem_socket].as<validators::nonempty_string>());
  }

  if( arrMasterNames.empty() )
    arrMasterNames.push_back("orchestrator"); // default master name

  sdpa::master_info_list_t listMasterInfo;

  if (not pidfile.empty())
  {
    fhg::util::pidfile_writer const pidfile_writer (pidfile);

    if (vm.count ("daemonize"))
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent
        ({&remote_log_io_service});
    }

    pidfile_writer.write();
  }
  else
  {
    if (vm.count ("daemonize"))
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent
        ({&remote_log_io_service});
    }
  }

  {
    std::stringstream startup_message;
    startup_message << "Starting agent '" << agentName
                    << "' at '" << agentUrl
                    << "', having masters: ";

    for (const std::string& master : arrMasterNames)
    {
      startup_message << master << ", ";
      listMasterInfo.push_back (sdpa::MasterInfo (master));
    }
  }

  boost::asio::io_service gui_io_service;
  boost::asio::io_service peer_io_service;
  boost::asio::io_service kvs_client_io_service;
  const sdpa::daemon::Agent agent
    ( agentName
    , agentUrl
    , peer_io_service
    , kvs_client_io_service
    , vm["kvs-host"].as<std::string>()
    , vm["kvs-port"].as<std::string>()
    , vmem_socket
    , listMasterInfo
    , std::pair<std::string, boost::asio::io_service&> (appGuiUrl, gui_io_service)
    );

  fhg::util::thread::event<> stop_requested;
  const std::function<void()> request_stop
    (std::bind (&fhg::util::thread::event<>::notify, &stop_requested));

  fhg::util::signal_handler_manager signal_handlers;

  signal_handlers.add_log_backtrace_and_exit_for_critical_errors (logger);

  signal_handlers.add (SIGTERM, std::bind (request_stop));
  signal_handlers.add (SIGINT, std::bind (request_stop));


  stop_requested.wait();
}
