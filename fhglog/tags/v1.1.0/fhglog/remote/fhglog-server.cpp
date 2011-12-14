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
#include <cstdlib>

#include <fhglog/remote/LogServer.hpp>
#include <fhglog/NullAppender.hpp>
#include <fhglog/Configuration.hpp>

int main(int argc, char **argv)
{
  try
  {
    std::string mode("start");
    short port(FHGLOG_DEFAULT_PORT);
    if (argc > 1)
    {
      const std::string a1(argv[1]);
      if (a1 == "-h")
      {
        std::cerr << "Usage: " << argv[0] << " [{start|stop} port]" << std::endl;
        std::cerr << "\tport - udp port to listen on (default:" << FHGLOG_DEFAULT_PORT << ")" << std::endl;
        return 1;
      }
      else if (a1 == "stop")
      {
        mode = a1;
      }
      else if (a1 == "start")
      {
        mode = a1;
      }
    }
    
    if (argc > 2)
    {
      std::istringstream isstr(argv[2]);
      isstr >> port;
      if (! isstr)
      {
        std::cerr << "invalid port specified: " << argv[1] << std::endl;
        return 2;
      }
    }

    fhg::log::Configurator::configure();

    if (mode == "start")
    {
      boost::asio::io_service io_service;

      fhg::log::Appender::ptr_t appender(new fhg::log::StreamAppender("console", std::cout));
      appender->setFormat(fhg::log::Formatter::Full());
      fhg::log::remote::LogServer server(appender, io_service, port);

      io_service.run();
      LOG(INFO, "done.");
      return 0;
    }
    else if (mode == "stop")
    {
      std::cerr << "stopping server on port " << port << std::endl;
      std::cerr << "\tFIXME: implement me correctly" << std::endl;
      std::ostringstream cmd;
      cmd << "echo -n QUIT deadbeef | nc -q 0 -u localhost " << port;
      return system(cmd.str().c_str());
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
