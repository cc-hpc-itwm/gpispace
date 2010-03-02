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
#include <csignal>

#include <fhglog/fhglog.hpp>
#include <fhglog/util.hpp>
#include <fhglog/remote/LogServer.hpp>

boost::asio::io_service io_service;

void signal_handler(int)
{
  io_service.stop();
}

int main(int argc, char **argv)
{
  try
  {
    std::string port_string(FHGLOG_DEFAULT_PORT);
    std::string fmt_string("full");

    if (argc > 1)
    {
      const std::string a1(argv[1]);
      if (a1 == "-h")
      {
        std::cerr << "Usage: " << argv[0] << "[fmt [port]]" << std::endl;
        std::cerr << "\tfmt  - format to use {short, full, custom} (default: full)" << std::endl;
        std::cerr << "\tport - udp port to listen on (default:" << FHGLOG_DEFAULT_PORT << ")" << std::endl;
        return 1;
      }
      else
      {
        fmt_string = a1;
      }
    }

    if (argc > 2)
    {
      port_string = argv[2];
    }

    unsigned short port;
    std::istringstream isstr(port_string);
    isstr >> port;
    if (! isstr)
    {
      std::cerr << "invalid port specified: " << argv[1] << std::endl;
      return 2;
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // my own output goes to stderr
    fhg::log::getLogger().addAppender(fhg::log::Appender::ptr_t(new fhg::log::StreamAppender("console", std::cerr)))->setFormat(fhg::log::Formatter::Default());

    // remote messages go to stdout
    fhg::log::Appender::ptr_t appender(new fhg::log::StreamAppender("console", std::cout));

    if      (fmt_string == "full")  appender->setFormat(fhg::log::Formatter::Full());
    else if (fmt_string == "short") appender->setFormat(fhg::log::Formatter::Short());
    else                            appender->setFormat(fhg::log::Formatter::Custom(fmt_string));

    fhg::log::remote::LogServer server(appender, io_service, port);

    io_service.run();
    LOG(INFO, "done.");
  }
  catch (const std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
