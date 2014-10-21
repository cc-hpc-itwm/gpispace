#include <fhglog/LogMacros.hpp>

#include <stdlib.h>
#include <csignal>

#include <iostream>
#include <boost/program_options.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/tcp_server.hpp>

#include <fhg/util/daemonize.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/pidfile_writer.hpp>
#include <fhg/syscall.hpp>


int main(int ac, char *av[])
{
  boost::asio::io_service remote_log_io_service;
  FHGLOG_SETUP (remote_log_io_service);

  fhg::log::Logger::ptr_t logger (fhg::log::Logger::get ("fhgkvsd"));

  namespace po = boost::program_options;

  std::string pidfile;
  bool daemonize = false;

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("bind", po::value<std::string>()->required(), "bind to this address")
    ("port", po::value<std::string>()->required(), "port or service name to use")
    ("pidfile", po::value<std::string>(&pidfile)->required(), "write pid to pidfile (required)")
    ("daemonize", "daemonize after all checks were successful")
    ;

  po::variables_map vm;
  try
  {
    po::store (po::parse_command_line (ac, av, desc), vm);

    if (vm.count ("help"))
    {
      std::cerr << "usage: " << av[0] << std::endl;
      std::cerr << std::endl;
      std::cerr << desc << std::endl;
      return EXIT_SUCCESS;
    }

    po::notify (vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    std::cerr << "try " << av[0] << " -h to get some help" << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count ("daemonize"))
    daemonize = true;

  if (pidfile.empty())
  {
    std::cerr << "parameter to --pidfile is empty!" << std::endl
              << "it has to point to a file that one can write to - if the file doesn't exist, it will be created."
              << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    boost::asio::io_service io_service;
    fhg::util::signal_handler_manager signal_handler;
    signal_handler.add_log_backtrace_and_exit_for_critical_errors (logger);

    signal_handler.add (SIGTERM, [&io_service] (int, siginfo_t*, void*) { io_service.stop(); });
    signal_handler.add (SIGINT, [&io_service] (int, siginfo_t*, void*) { io_service.stop(); });

    fhg::util::pidfile_writer pidfile_writer (pidfile);

    fhg::com::kvs::server::kvsd kvsd;

    fhg::com::tcp_server server ( io_service
                                , kvsd
                                , vm["bind"].as<std::string>() == "*"
                                ? "0"
                                : vm["bind"].as<std::string>()
                                , vm["port"].as<std::string>()
                                , true
                                );

    std::cout << server.port() << std::endl;

    if (daemonize)
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent
        ({&remote_log_io_service, &io_service});
    }

    pidfile_writer.write();

    io_service.run();

    if (not pidfile.empty())
    {
      fhg::syscall::unlink (pidfile.c_str());
    }
  }
  catch (std::exception const& ex)
  {
    std::cerr << "could not start server: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  return 0;
}
