// chat server example from boost.org
//    http://www.boost.org/doc/libs/1_43_0/doc/html/boost_asio/example/chat/chat_server.cpp

#include <fhglog/fhglog.hpp>

#include <deque>
#include <iostream>
#include <list>
#include <set>
#include <sstream>

#include <boost/program_options.hpp>
#include <boost/bind.hpp>

#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_client.hpp>

using boost::asio::ip::tcp;

int main(int ac, char *av[])
{
  FHGLOG_SETUP(ac,av);
  namespace po = boost::program_options;

  std::string server_address ("localhost");
  std::string server_port ("2439");

  po::options_description desc ("options");
  desc.add_options()
    ("help,h", "print this help")
    ("server,s", po::value<std::string>(&server_address)->default_value(server_address), "use this server")
    ("port,p", po::value<std::string>(&server_port)->default_value(server_port), "port or service name to use")
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

  // fork service thread
  fhg::com::io_service_pool pool (1);
  boost::thread thrd (boost::bind ( &fhg::com::io_service_pool::run
                                  , &pool
                                  )
                     );

  fhg::com::tcp_client client ( pool.get_io_service()
                              , server_address
                              , server_port
                              );
  client.start ();

  while ( std::cin )
  {
    std::cerr << "reading message..." << std::endl;
    std::string input;
    std::getline (std::cin, input);
    if (input.empty())
      continue;
    else if (input == "quit")
      break;
    std::cerr << "sending " << input << std::endl;
    client.send (input);
  }

  client.stop();
  pool.stop();

  return 0;
}
