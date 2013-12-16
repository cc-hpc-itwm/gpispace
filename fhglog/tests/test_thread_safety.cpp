// alexander.petry@itwm.fraunhofer.de

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/SynchronizedAppender.hpp>
#include <unistd.h> // usleep

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

namespace
{
  void thread_function (std::string logger_name, std::size_t loop_count)
  {
    boost::this_thread::yield();

    for (std::size_t i (0); i < loop_count; ++i)
    {
      fhg::log::getLogger (logger_name).log
        (FHGLOG_MKEVENT_HERE (DEBUG, boost::lexical_cast<std::string> (i)));

      if (0 == (i % 50))
      {
        boost::this_thread::sleep (boost::posix_time::microseconds (100));
      }
    }
  }
}

int main (int , char **)
{
  std::ostringstream logstream;

  fhg::log::logger_t root (fhg::log::getLogger());
  root.addAppender(fhg::log::Appender::ptr_t(new fhg::log::SynchronizedAppender(new fhg::log::StreamAppender("stringstream", logstream, "%m"))));

  {
    std::clog << "** testing multiple threads...";
    std::size_t loop_count = 1000;

    boost::thread t0 (&thread_function, "0", loop_count);
    boost::thread t1 (&thread_function, "0", loop_count);
    boost::thread t2 (&thread_function, "0", loop_count);

    if (t2.joinable()) { t2.join(); }
    if (t1.joinable()) { t1.join(); }
    if (t0.joinable()) { t0.join(); }

    // didn't segfault...
  }
}
