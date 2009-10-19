/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  log server daemon
 *
 *        Version:  1.0
 *        Created:  10/19/2009 09:31:40 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream>
#include <fhglog/remote/LogServer.hpp>
#include <fhglog/NullAppender.hpp>
#include <fhglog/Configuration.hpp>

int main(int argc, char **argv)
{
  fhg::log::Configurator::configure();

  try
  {
    short port(FHGLOG_DEFAULT_PORT);
    if (argc > 1)
    {
      if (std::string(argv[1]) == "-h")
      {
        std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
        std::cerr << "\tport - udp port to listen on (default:" << FHGLOG_DEFAULT_PORT << ")" << std::endl;
        return 1;
      }
      std::istringstream isstr(argv[1]);
      isstr >> port;
      if (isstr.bad())
      {
        std::cerr << "invalid port specified: " << argv[1] << std::endl;
        return 2;
      }
    }

    boost::asio::io_service io_service;
    fhg::log::Appender::ptr_t null(new fhg::log::NullAppender());

    fhg::log::remote::LogServer server(null, io_service, port);

    LOG(INFO, "entering event loop...");
    io_service.run();
    LOG(INFO, "done.");
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
