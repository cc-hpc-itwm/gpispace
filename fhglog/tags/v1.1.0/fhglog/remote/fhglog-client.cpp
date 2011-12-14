/*
 * =====================================================================================
 *
 *       Filename:  fhglog-client.cpp
 *
 *    Description:  send messages to an fhg-log daemon
 *
 *        Version:  1.0
 *        Created:  10/19/2009 02:24:52 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/remote/RemoteAppender.hpp>

void usage()
{
  std::cerr << "usage: fhglog-client [-h] message [server [port]]" << std::endl;
  std::cerr << "\tmessage - the message to log" << std::endl;
  std::cerr << "\tserver - server to use (default: " << FHGLOG_DEFAULT_HOST << ")" << std::endl;
  std::cerr << "\tport   - port to send events to (default: " << FHGLOG_DEFAULT_PORT << ")" << std::endl;
}

int main (int argc, char **argv)
{
  using namespace fhg::log;

  std::string message("hello server!");
  std::string server(FHGLOG_DEFAULT_HOST);
  short port(FHGLOG_DEFAULT_PORT);

  if (argc <= 1)
  {
    usage();
    return 1;
  }

  if (argc > 1)
  {
    if (std::string(argv[1]) == "-h")
    {
      usage();
      return 0;
    }
    message = argv[1];
  }

  if (argc > 2)
  {
    server = argv[2];
  }

  if (argc > 3)
  {
    std::istringstream isstr(argv[3]);
    isstr >> port;
    if (! isstr)
    {
      usage();
      std::cerr << "invalid port specified: " << argv[3] << std::endl;
      return 1;
    }
  }

  logger_t log(getLogger());
  log.addAppender(Appender::ptr_t(new remote::RemoteAppender("remote", server, port)))->setFormat(Formatter::Custom("%m%n"));
  log.log(FHGLOG_MKEVENT_HERE(INFO, message));
  return 0;
}
