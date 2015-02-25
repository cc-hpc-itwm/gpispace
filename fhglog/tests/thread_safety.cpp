// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE thread_safety
#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhglog/format.hpp>
#include <fhglog/appender/stream.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

namespace
{
  const std::size_t loop_count (10000);

  void thread_function (fhg::log::Logger* logger)
  {
    boost::this_thread::yield();

    for (std::size_t i (0); i < loop_count; ++i)
    {
      logger->log
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

  fhg::log::Logger l0;
  fhg::log::Logger l1;
  fhg::log::Logger l2;
  l0.addAppender<pushback_appender> (&output_0);
  l1.addAppender<pushback_appender> (&output_1);
  l2.addAppender<pushback_appender> (&output_2);

  {
    const boost::strict_scoped_thread<> t0 (&thread_function, &l0);
    const boost::strict_scoped_thread<> t1 (&thread_function, &l1);
    const boost::strict_scoped_thread<> t2 (&thread_function, &l2);
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

  fhg::log::Logger l;
  l.addAppender<pushback_appender_synchronized> (&output);

  {
    const boost::strict_scoped_thread<> t0 (&thread_function, &l);
    const boost::strict_scoped_thread<> t1 (&thread_function, &l);
    const boost::strict_scoped_thread<> t2 (&thread_function, &l);
  }

  std::sort (output.begin(), output.end());

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (output.begin(), output.end(), reference.begin(), reference.end());
}
