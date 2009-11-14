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
  std::cerr << "usage: fhglog-client [-h] ip:port message..." << std::endl;
  std::cerr << "\thost:port - server to use (default: " << FHGLOG_DEFAULT_HOST << ")" << std::endl;
  std::cerr << "\tmessage - the message to log" << std::endl;
}

int main (int argc, char **argv)
{
  using namespace fhg::log;

  std::string message("hello server!");
  std::string server(FHGLOG_DEFAULT_LOCATION);

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
    server = argv[1];
  }

  logger_t log(getLogger());
  log.addAppender(Appender::ptr_t(new remote::RemoteAppender("remote", server)));

  std::ostringstream msg;
  for (int i = 2; ;)
  {
    if (i == argc) break;

    msg << argv[i];
    ++i;

    if (i < argc)
      msg << " ";
  }

  log.log(FHGLOG_MKEVENT_HERE(INFO, msg.str()));
  return 0;
}
