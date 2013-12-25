// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE thread_safety
#include <boost/test/unit_test.hpp>

#include <fhglog/fhglog.hpp>
#include <fhglog/format.hpp>
#include <fhglog/appender/stream.hpp>
#include <fhglog/appender/synchronized.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

namespace
{
  const std::size_t loop_count (1000);

  void thread_function (std::string logger_name)
  {
    boost::this_thread::yield();

    for (std::size_t i (0); i < loop_count; ++i)
    {
      fhg::log::getLogger (logger_name).log
        (FHGLOG_MKEVENT_HERE (ERROR, boost::lexical_cast<std::string> (i)));

      if (0 == (i % 50))
      {
        boost::this_thread::sleep (boost::posix_time::microseconds (100));
      }
    }
  }

  struct pushback_appender : public fhg::log::Appender
  {
    pushback_appender (std::vector<std::string>* container)
      : _container (container)
    {}

    void append (const fhg::log::LogEvent &evt)
    {
      _container->push_back (fhg::log::format ("%m", evt));
    }
    void flush () {}

    std::vector<std::string>* _container;
  };

  struct counter
  {
    counter() : _ (0) { }
    std::string operator()() { return boost::lexical_cast<std::string> (_++); }
    int _;
  };
}

BOOST_AUTO_TEST_CASE (different_loggers)
{
  std::vector<std::string> reference (loop_count);
  std::generate (reference.begin(), reference.end(), counter());

  std::vector<std::string> output_0;
  std::vector<std::string> output_1;
  std::vector<std::string> output_2;

  fhg::log::getLogger ("0").addAppender
    (fhg::log::Appender::ptr_t (new pushback_appender (&output_0)));
  fhg::log::getLogger ("1").addAppender
    (fhg::log::Appender::ptr_t (new pushback_appender (&output_1)));
  fhg::log::getLogger ("2").addAppender
    (fhg::log::Appender::ptr_t (new pushback_appender (&output_2)));

  boost::thread t0 (&thread_function, "0");
  boost::thread t1 (&thread_function, "1");
  boost::thread t2 (&thread_function, "2");

  if (t2.joinable()) { t2.join(); }
  if (t1.joinable()) { t1.join(); }
  if (t0.joinable()) { t0.join(); }

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (output_0.begin(), output_0.end(), reference.begin(), reference.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (output_1.begin(), output_1.end(), reference.begin(), reference.end());
  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (output_2.begin(), output_2.end(), reference.begin(), reference.end());
}

BOOST_AUTO_TEST_CASE (same_logger)
{
  std::vector<std::string> reference (3 * loop_count);
  std::generate_n (reference.begin() + 0 * loop_count, loop_count, counter());
  std::generate_n (reference.begin() + 1 * loop_count, loop_count, counter());
  std::generate_n (reference.begin() + 2 * loop_count, loop_count, counter());
  std::sort (reference.begin(), reference.end());

  std::vector<std::string> output;

  fhg::log::Appender::ptr_t sync_appender
    (new fhg::log::SynchronizedAppender (new pushback_appender (&output)));
  fhg::log::getLogger ("log").addAppender (sync_appender);

  boost::thread t0 (&thread_function, "log");
  boost::thread t1 (&thread_function, "log");
  boost::thread t2 (&thread_function, "log");

  if (t2.joinable()) { t2.join(); }
  if (t1.joinable()) { t1.join(); }
  if (t0.joinable()) { t0.join(); }

  std::sort (output.begin(), output.end());

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (output.begin(), output.end(), reference.begin(), reference.end());
}
