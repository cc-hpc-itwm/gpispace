#define BOOST_TEST_MODULE event
#include <boost/test/unit_test.hpp>

#include <fhglog/LogEvent.hpp>
#include <fhglog/LogLevel.hpp>

//! \todo This should test way more
BOOST_AUTO_TEST_CASE (event_ctor_should_regard_severity)
{
  const fhg::log::LogEvent evt
    (fhg::log::LogLevel::DEBUG, "file", "function", 34, "hello world!");
  BOOST_REQUIRE_EQUAL (evt.severity().lvl(), fhg::log::LogLevel::DEBUG);
}

BOOST_AUTO_TEST_CASE (illegal_loglevel_should_throw)
{
  BOOST_REQUIRE_THROW
    ( fhg::log::LogLevel
      (static_cast<fhg::log::LogLevel::Level> (fhg::log::LogLevel::MAX_LEVEL+1))
    , std::runtime_error
    );
}

//! \todo This does not really test anything
BOOST_AUTO_TEST_CASE (ctor_should_not_throw_and_should_be_copyable)
{
  const fhg::log::LogEvent evt1
    (fhg::log::LogLevel::DEBUG, "tests/test_event.cpp", "main", 34, "hello world!");
  const fhg::log::LogEvent evt2 (evt1);
  const fhg::log::LogEvent evt3
    (fhg::log::LogLevel::INFO, "tests/test_formatter.cpp", "foo", 42, "blah!");
}
