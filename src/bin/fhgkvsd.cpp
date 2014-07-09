#include <fhglog/LogMacros.hpp>

#include <stdlib.h>
#include <csignal>

#include <iostream>
#include <boost/program_options.hpp>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/peer_info.hpp>
#include <fhgcom/tcp_server.hpp>

#include <fhg/util/daemonize.hpp>
#include <fhg/util/pidfile_writer.hpp>
#include <fhg/syscall.hpp>


int main(int ac, char *av[])
{
  FHGLOG_SETUP();

  namespace po = boost::program_options;

  std::string server_address ("*");
  std::string server_port ("2439");
  std::string pidfile;
  bool daemonize = false;

  if (getenv("KVS_URL") != nullptr)
  {
    try
    {
      using namespace fhg::com;
      peer_info_t pi (getenv("KVS_URL"));
      server_address = pi.host(server_address);
      server_port = pi.port(server_port);
    }
    catch (std::exception const & ex)
    {
      std::cerr << "W: malformed environment variable KVS_URL: " << ex.what() << std::endl;
    }
  }

  bool reuse_address (true);
  std::string store_path;

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("bind,b", po::value<std::string>(&server_address)->default_value(server_address), "bind to this address")
    ("port,p", po::value<std::string>(&server_port)->default_value(server_port), "port or service name to use")
    ("reuse-address", po::value<bool>(&reuse_address)->default_value(reuse_address), "reuse address")
    ("store,s", po::value<std::string>(&store_path), "path to persistent store")
    ("clear,C", "start with an empty store")
    ("pidfile", po::value<std::string>(&pidfile), "write pid to pidfile (required)")
    ("daemonize", "daemonize after all checks were successful")
    ;

  po::variables_map vm;
  try
  {
    po::store (po::parse_command_line (ac, av, desc), vm);
    po::notify (vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    std::cerr << "try " << av[0] << " -h to get some help" << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count ("help"))
  {
    std::cerr << "usage: " << av[0] << std::endl;
    std::cerr << std::endl;
    std::cerr << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count ("daemonize"))
    daemonize = true;

  if (server_address == "*")
  {
    server_address = "0";
  }

  if (pidfile.empty())
  {
    std::cerr << "pidfile '" << pidfile << "'"
              << " is invalid, please specify --pidfile=<file> correctly"
              << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    boost::asio::io_service io_service;
    boost::optional<std::string> const optional_store_path (! store_path.empty (), store_path);
    fhg::com::kvs::server::kvsd kvsd (optional_store_path);

    if (vm.count ("clear")) kvsd.clear ("");

    boost::asio::signal_set term_signals (io_service, SIGINT, SIGTERM);
    term_signals.async_wait
      (boost::bind (&boost::asio::io_service::stop, &io_service));
    fhg::util::pidfile_writer pidfile_writer (pidfile);

    boost::asio::signal_set hup_signal (io_service, SIGHUP);
    hup_signal.async_wait
      (boost::bind (&fhg::com::kvs::server::kvsd::clear, &kvsd, ""));

    boost::asio::signal_set usr1_signal (io_service, SIGUSR1);
    usr1_signal.async_wait
      (boost::bind (&fhg::com::kvs::server::kvsd::save, &kvsd));

    boost::asio::signal_set usr2_signal (io_service, SIGUSR2);
    usr2_signal.async_wait
      (boost::bind (&fhg::com::kvs::server::kvsd::load, &kvsd));

    fhg::com::tcp_server server ( io_service
                                , kvsd
                                , server_address
                                , server_port
                                , reuse_address
                                );

    io_service.notify_fork (boost::asio::io_service::fork_prepare);
    if (daemonize)
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent();
    }

    pidfile_writer.write();

    io_service.notify_fork(boost::asio::io_service::fork_child);
    io_service.run();

    kvsd.save();

    if (not pidfile.empty ())
    {
      fhg::syscall::unlink (pidfile.c_str());
    }
  }
  catch (std::exception const &ex)
  {
    std::cerr << "could not start server: " << ex.what () << std::endl;
    return EXIT_FAILURE;
  }

  return 0;
}
