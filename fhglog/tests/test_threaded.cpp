// alexander.petry@itwm.fraunhofer.de

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/CompoundAppender.hpp>
#include <fhglog/ThreadedAppender.hpp>

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t log(getLogger());
  log.setLevel(LogLevel::MIN_LEVEL);

  std::ostringstream logstream;

  {
    std::clog << "** testing event appending (one appender, threaded)...";

        Appender::ptr_t s1(new StreamAppender("s1", logstream, "%m"));

	ThreadedAppender::ptr_t threaded_appender(new ThreadedAppender(s1));

	log.addAppender(threaded_appender);

	const std::string msg("hello world!");
    log.log(FHGLOG_MKEVENT_HERE(DEBUG, msg));

	threaded_appender->flush(); // wait for completion

    if (logstream.str() != msg)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlogged message: \"" << logstream.str() << "\"" << std::endl;
      std::clog << "\texpected: \"" << msg << "\"" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
    logstream.str("");
	log.removeAppender(threaded_appender->name());
  }

  log.removeAllAppenders();
  return errcount;
}
