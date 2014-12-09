#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <fhglog/LogMacros.hpp>

#include <fhg/util/boost/program_options/validators/existing_path.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/program_options.hpp>
#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <boost/filesystem/path.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/tokenizer.hpp>

#include <functional>

namespace bfs = boost::filesystem;
namespace po = boost::program_options;

int main (int argc, char **argv)
try
{
    std::string orchName;
    std::string orchUrl;

  boost::asio::io_service remote_log_io_service;
  FHGLOG_SETUP (remote_log_io_service);

    po::options_description desc("Allowed options");
    desc.add_options()
       ("help,h", "Display this message")
       ("name,n", po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
       ("url,u",  po::value<std::string>(&orchUrl)->default_value("localhost"), "Orchestrator's url")
       ( "startup-messages-pipe"
       , po::value<int>()->required()
       , "pipe filedescriptor to use for communication during startup (ports used, ...)"
       );

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

  boost::asio::io_service peer_io_service;
  boost::asio::io_service rpc_io_service;
  const sdpa::daemon::Orchestrator orchestrator
    ( orchName
    , orchUrl
    , peer_io_service
    , rpc_io_service
    );

  fhg::util::thread::event<> stop_requested;
  const std::function<void()> request_stop
    (std::bind (&fhg::util::thread::event<>::notify, &stop_requested));

  fhg::util::signal_handler_manager signal_handlers;

  signal_handlers.add_log_backtrace_and_exit_for_critical_errors (logger);

  signal_handlers.add (SIGTERM, std::bind (request_stop));
  signal_handlers.add (SIGINT, std::bind (request_stop));

  {
    boost::iostreams::stream<boost::iostreams::file_descriptor_sink>
      startup_messages_pipe ( vm["startup-messages-pipe"].as<int>()
                            , boost::iostreams::close_handle
                            );
    startup_messages_pipe << orchestrator.peer_local_endpoint().address().to_string() << "\n";
    startup_messages_pipe << orchestrator.peer_local_endpoint().port() << "\n";
    startup_messages_pipe << orchestrator.rpc_local_endpoint().address().to_string() << "\n";
    startup_messages_pipe << orchestrator.rpc_local_endpoint().port() << "\n";
    startup_messages_pipe << "OKAY\n";
  }

  stop_requested.wait();
}
catch (std::exception const& ex)
{
  std::cerr << "EXCEPTION: " << ex.what() << std::endl;
  return 1;
}
