// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE thread_safety
#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhglog/format.hpp>
#include <fhglog/appender/stream.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

namespace
{
  const std::size_t loop_count (10000);

  void thread_function (std::string logger_name)
  {
    boost::this_thread::yield();

    for (std::size_t i (0); i < loop_count; ++i)
    {
      fhg::log::Logger::get (logger_name)->log
        (FHGLOG_MKEVENT_HERE (ERROR, boost::lexical_cast<std::string> (i)));
    }
  }

  struct pushback_appender : public fhg::log::Appender
  {
    pushback_appender (std::vector<std::string>* container)
      : _container (container)
    {}

    virtual void append (const fhg::log::LogEvent &evt) override
    {
      _container->push_back (fhg::log::format ("%m", evt));
    }
    virtual void flush () override {}

    std::vector<std::string>* _container;
  };

  struct pushback_appender_synchronized : public fhg::log::Appender
  {
    pushback_appender_synchronized (std::vector<std::string>* container)
      : _container (container)
    {}

    virtual void append (const fhg::log::LogEvent &evt) override
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex);

      _container->push_back (fhg::log::format ("%m", evt));
    }
    virtual void flush () override {}

    std::vector<std::string>* _container;
    boost::recursive_mutex _mutex;
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

  fhg::log::Logger::get ("0")->addAppender
    (fhg::log::Appender::ptr_t (new pushback_appender (&output_0)));
  fhg::log::Logger::get ("1")->addAppender
    (fhg::log::Appender::ptr_t (new pushback_appender (&output_1)));
  fhg::log::Logger::get ("2")->addAppender
    (fhg::log::Appender::ptr_t (new pushback_appender (&output_2)));

  {
    const boost::strict_scoped_thread<> t0 (&thread_function, "0");
    const boost::strict_scoped_thread<> t1 (&thread_function, "1");
    const boost::strict_scoped_thread<> t2 (&thread_function, "2");
  }

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
    (fhg::log::Appender::ptr_t (new pushback_appender_synchronized (&output)));
  fhg::log::Logger::get ("log")->addAppender (sync_appender);

  {
    const boost::strict_scoped_thread<> t0 (&thread_function, "log");
    const boost::strict_scoped_thread<> t1 (&thread_function, "log");
    const boost::strict_scoped_thread<> t2 (&thread_function, "log");
  }

  std::sort (output.begin(), output.end());

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (output.begin(), output.end(), reference.begin(), reference.end());
}
