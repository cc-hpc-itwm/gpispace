// alexander.petry@itwm.fraunhofer.de

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/FilteringAppender.hpp>
#include <fhglog/Filter.hpp>

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);

  std::ostringstream logstream;
  Appender::ptr_t streamAppender(new StreamAppender("stream", logstream, "%m"));

  {
    std::clog << "** testing level filter with filtering appender...";
    Filter::ptr_t filter(new LevelFilter(LogLevel::ERROR));
    Appender::ptr_t appender(new FilteringAppender(streamAppender, filter));

    appender->append(FHGLOG_MKEVENT_HERE(DEBUG, "hello world!"));
    if (! logstream.str().empty())
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlogged message: " << logstream.str() << std::endl;
      std::clog << "\tnothing expected!" << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
    logstream.str("");
  }

  return errcount;
}
