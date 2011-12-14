/*
 * =====================================================================================
 *
 *       Filename:  test_remote_logging.cpp
 *
 *    Description:  
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

int main (int argc, char **argv)
{
  using namespace fhg::log;

  int errcount(0);

  std::string message("hello server!");
  std::string server("localhost");
  short port(FHGLOG_DEFAULT_PORT);

  if (argc > 1)
  {
    if (std::string(argv[1]) == "-h")
    {
      std::cerr << "usage: " << argv[0] << " [-h] [server [port]]" << std::endl;
      std::cerr << "\tserver - server to use (default: " << server << ")" << std::endl;
      std::cerr << "\tport   - port to send events to (default: " << port << ")" << std::endl;
      return 0;
    }
    server = argv[1];
  }

  if (argc > 2)
  {
    port = atoi(argv[2]);
  }

  if (argc > 3)
  {
    message = argv[3];
  }

  logger_t log(getLogger());
  log.addAppender(Appender::ptr_t(new remote::RemoteAppender("remote", server, port)))->setFormat(Formatter::Full());

  {
    std::clog << "** testing remote logging (TODO: create server socket)...";
    log.log(FHGLOG_MKEVENT_HERE(DEBUG, message));
    std::clog << "OK!" << std::endl;
  }

  return errcount;
}
