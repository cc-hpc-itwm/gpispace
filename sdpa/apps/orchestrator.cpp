#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <fhglog/LogMacros.hpp>

#include <boost/program_options.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <boost/filesystem/path.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/pidfile_writer.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/tokenizer.hpp>

#include <functional>

namespace bfs = boost::filesystem;
namespace po = boost::program_options;

int main (int argc, char **argv)
{
    std::string orchName;
    std::string orchUrl;
    std::string pidfile;

  boost::asio::io_service remote_log_io_service;
  FHGLOG_SETUP (remote_log_io_service);

    po::options_description desc("Allowed options");
    desc.add_options()
       ("help,h", "Display this message")
       ("name,n", po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
       ("url,u",  po::value<std::string>(&orchUrl)->default_value("localhost"), "Orchestrator's url")
       ("kvs-host",  po::value<std::string>()->required(), "The kvs daemon's host")
       ("kvs-port",  po::value<std::string>()->required(), "The kvs daemon's port")
       ("pidfile", po::value<std::string>(&pidfile)->default_value(pidfile), "write pid to pidfile")
       ("daemonize", "daemonize after all checks were successful")
       ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

    fhg::log::Logger::ptr_t logger (fhg::log::Logger::get (orchName));

    if (vm.count("help"))
    {
      LLOG (ERROR, logger, "usage: orchestrator [options] ....");
      LLOG (ERROR, logger, desc);
      return 0;
    }

    po::notify(vm);

    if (not pidfile.empty())
    {
      fhg::util::pidfile_writer const pidfile_writer (pidfile);

      if (vm.count ("daemonize"))
      {
        fhg::util::fork_and_daemonize_child_and_abandon_parent();
      }

      pidfile_writer.write();
    }
    else
    {
      if (vm.count ("daemonize"))
      {
        fhg::util::fork_and_daemonize_child_and_abandon_parent();
      }
    }

  boost::asio::io_service peer_io_service;
  boost::asio::io_service kvs_client_io_service;
  const sdpa::daemon::Orchestrator orchestrator
    ( orchName
    , orchUrl
    , peer_io_service
    , kvs_client_io_service
    , vm["kvs-host"].as<std::string>()
    , vm["kvs-port"].as<std::string>()
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
