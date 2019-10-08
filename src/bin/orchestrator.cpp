#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <csignal>

#include <fhglog/Configuration.hpp>
#include <fhglog/LogMacros.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/getenv.hpp>
#include <util-generic/print_exception.hpp>

#include <rif/started_process_promise.hpp>

#include <boost/program_options.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>
#include <boost/filesystem/path.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <functional>

namespace bfs = boost::filesystem;
namespace po = boost::program_options;

namespace
{
  namespace option_name
  {
    constexpr const char* ssl_certificates {"ssl-certificates"};
  }
}

int main (int argc, char **argv)
{
  fhg::rif::started_process_promise promise (argc, argv);

  try
  {
    std::string orchName;
    std::string orchUrl;
    fhg::com::Certificates ssl_certificates;

    boost::asio::io_service remote_log_io_service;
    fhg::log::Logger logger;
    fhg::log::configure
      ( logger
      , remote_log_io_service
      , fhg::util::getenv ("FHGLOG_level").get()
      , fhg::util::getenv ("FHGLOG_to_file")
      , fhg::util::getenv ("FHGLOG_to_server")
      );

    po::options_description desc("Allowed options");
    desc.add_options()
       ("help,h", "Display this message")
       ("name,n", po::value<std::string>(&orchName)->default_value("orchestrator"), "Orchestrator's logical name")
       ("url,u",  po::value<std::string>(&orchUrl)->default_value("localhost"), "Orchestrator's url")
       ( option_name::ssl_certificates
       , po::value<bfs::path>()
       , "folder containing SSL certificates"
       )
      ;

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

    if (vm.count("help"))
    {
      LLOG (ERROR, logger, "usage: orchestrator [options] ....");
      LLOG (ERROR, logger, desc);
      return 0;
    }

    po::notify(vm);

    if (vm.count (option_name::ssl_certificates))
    {
      ssl_certificates = vm.at (option_name::ssl_certificates).as<bfs::path>();
    }

    const sdpa::daemon::GenericDaemon orchestrator
      ( orchName
      , orchUrl
      , fhg::util::cxx14::make_unique<boost::asio::io_service>()
      , boost::none
      , sdpa::master_info_t()
      , logger
      , false
      , ssl_certificates
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

    promise.set_result ( fhg::util::connectable_to_address_string
                           (orchestrator.peer_local_endpoint().address())
                       , std::to_string
                           (orchestrator.peer_local_endpoint().port())
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
