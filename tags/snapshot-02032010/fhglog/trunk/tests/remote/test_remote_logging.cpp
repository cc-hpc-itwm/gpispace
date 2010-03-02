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

int main ()
{
  using namespace fhg::log;

  int errcount(0);

  std::string message("hello server!");
  std::string server(FHGLOG_DEFAULT_LOCATION);

  logger_t log(getLogger());
//  log.addAppender(Appender::ptr_t(new remote::RemoteAppender("remote", server)))->setFormat(Formatter::Full());
  log.addAppender(Appender::ptr_t(new remote::RemoteAppender("remote", server)));

  {
    std::clog << "** testing remote logging (TODO: create server socket)...";
    log.log(FHGLOG_MKEVENT_HERE(DEBUG, message));
    std::clog << "OK!" << std::endl;
  }

  return errcount;
}
