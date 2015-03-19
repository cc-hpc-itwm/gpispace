#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <fhglog/Configuration.hpp>
#include <fhglog/LogMacros.hpp>

#include <fhg/util/boost/asio/ip/address.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/make_unique.hpp>
#include <util-generic/print_exception.hpp>

#include <rif/startup_messages_pipe.hpp>

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
  fhg::log::configure (remote_log_io_service, fhg::log::GLOBAL_logger());

    po::options_description desc("Allowed options");
    desc.add_options()
       ("help,h", "Display this message")
       ("name,n", po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
       ("url,u",  po::value<std::string>(&orchUrl)->default_value("localhost"), "Orchestrator's url")
      ;

    desc.add (fhg::rif::startup_messages_pipe::program_options());

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

    fhg::log::Logger& logger (fhg::log::GLOBAL_logger());

    if (vm.count("help"))
    {
      LLOG (ERROR, logger, "usage: orchestrator [options] ....");
      LLOG (ERROR, logger, desc);
      return 0;
    }

    po::notify(vm);

  boost::asio::io_service rpc_io_service;
  const sdpa::daemon::Orchestrator orchestrator
    ( orchName
    , orchUrl
    , fhg::util::make_unique<boost::asio::io_service>()
    , rpc_io_service
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
    startup_messages_pipe << fhg::util::connectable_to_address_string
                               (orchestrator.peer_local_endpoint().address());
    startup_messages_pipe << orchestrator.peer_local_endpoint().port();
    startup_messages_pipe << fhg::util::connectable_to_address_string
                               (orchestrator.rpc_local_endpoint().address());
    startup_messages_pipe << orchestrator.rpc_local_endpoint().port();
  }

  stop_requested.wait();
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EXCEPTION: ");
  return 1;
}
