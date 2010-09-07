#include <fhglog/fhglog.hpp>

#include <iostream>
#include <boost/program_options.hpp>

#include <fhgcom/session_manager.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>
#include <fhgcom/session.hpp>

int main(int ac, char *av[])
{
  FHGLOG_SETUP(ac,av);

  namespace po = boost::program_options;

  std::string server_address ("");
  std::string server_port ("1234");
  bool reuse_address (true);

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("bind,b", po::value<std::string>(&server_address)->default_value(server_address), "bind to this address")
    ("port,p", po::value<std::string>(&server_port)->default_value(server_port), "port or service name to use")
    ("reuse-address", po::value<bool>(&reuse_address)->default_value(reuse_address), "reuse address")
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

  fhg::com::tcp_server::manager_t session_manager;
  fhg::com::io_service_pool pool (4);
  fhg::com::tcp_server server ( pool
                              , session_manager
                              , server_address
                              , server_port
                              , reuse_address
                              );

  server.start ();

  pool.run();
  return 0;
}
