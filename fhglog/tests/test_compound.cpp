// alexander.petry@itwm.fraunhofer.de

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/CompoundAppender.hpp>

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t log(getLogger());
  log.setLevel(LogLevel::MIN_LEVEL);

  std::ostringstream logstream;

  {
	CompoundAppender::ptr_t compound(new CompoundAppender("compound-appender"));

	compound->addAppender(Appender::ptr_t(new StreamAppender("s1", logstream, "%m")));
	compound->addAppender(Appender::ptr_t(new StreamAppender("s2", logstream, "%m")));

	log.addAppender(compound);

    std::clog << "** testing event appending (two appender combined)...";
	const std::string msg("hello world!");
    log.log(FHGLOG_MKEVENT_HERE(DEBUG, msg));
    if (logstream.str() != (msg + msg))
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlogged message: \"" << logstream.str() << "\"" << std::endl;
      std::clog << "\texpected: \"" << msg << msg << "\"" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
    logstream.str("");
	log.removeAppender("compound-appender");
  }

  log.removeAllAppenders();
  return errcount;
}
